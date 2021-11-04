#include "channel.hpp"
#include <boost/algorithm/string.hpp>
#include <boost/asio.hpp>
#include <boost/locale.hpp>
#include <boost/thread.hpp>
#include <cmath>
#include <iostream>
#include <map>
#include "internal/amqp.hpp"
#include "logger.hpp"


namespace is {

using namespace opentracing;

struct TextMapCarrier : TextMapReader, TextMapWriter {
  TextMapCarrier(std::unordered_map<std::string, std::string>& text_map_) : text_map(text_map_) {}

  expected<void> Set(string_view key, string_view value) const override {
    text_map[key] = value;
    return {};
  }

  expected<string_view> LookupKey(string_view key) const override {
    if (!supports_lookup) { return make_unexpected(lookup_key_not_supported_error); }
    auto iter = text_map.find(key);
    if (iter != text_map.end()) {
      return string_view{iter->second};
    } else {
      return make_unexpected(key_not_found_error);
    }
  }

  expected<void> ForeachKey(
      std::function<expected<void>(string_view key, string_view value)> f) const override {
    ++foreach_key_call_count;
    for (const auto& key_value : text_map) {
      auto result = f(key_value.first, key_value.second);
      if (!result) return result;
    }
    return {};
  }

  bool supports_lookup = false;
  mutable int foreach_key_call_count = 0;
  std::unordered_map<std::string, std::string>& text_map;
};

boost::shared_ptr<AmqpClient::Channel> get_impl(Subscription&, Channel& channel) {
  return channel._impl;
}

std::string get_exchange(Subscription&, Channel const& channel) {
  return channel._exchange;
}

Channel::Channel(std::string const& uri)
    : _impl(AmqpClient::Channel::CreateFromUri(uri)), _exchange("is") {}

void Channel::set_tracer(std::shared_ptr<opentracing::v1::Tracer> const& tracer) {
  _tracer = tracer;
}

std::shared_ptr<opentracing::v1::Tracer> Channel::tracer() const {
  return _tracer;
}

void Channel::publish(std::string const& topic, Message const& message) const {
  if (topic.empty()) warn("publishing to empty topic");

  uint64_t us_timestamp = std::chrono::duration_cast<std::chrono::microseconds>(
                              std::chrono::system_clock::now().time_since_epoch())
                              .count();
                              
  auto msg = to_internal_message(message);

  std::string str_timestamp = std::to_string(us_timestamp);

  AmqpClient::Table msgHeaders;

  msgHeaders = msg->HeaderTable();

  if (msgHeaders.find("timestamp_send") != msgHeaders.end()) {msgHeaders.erase(msgHeaders.find("timestamp_send")); }

  msgHeaders.insert(AmqpClient::TableEntry("timestamp_send", us_timestamp));
  msg->HeaderTable(msgHeaders);
  _impl->BasicPublish(_exchange, topic, msg);
}

void Channel::publish(Message const& message) const {
  publish(message.topic(), message);
}

void create_tracer(boost::shared_ptr<AmqpClient::Envelope> message, std::string const& commtrace_exporter_ip, std::uint16_t const& commtrace_exporter_port) {
  using namespace boost::asio;
  using ip::udp;
  io_service io_service;

  udp::socket socket(io_service);
  socket.open(udp::v4());
  socket.set_option(socket_base::broadcast(true));
  AmqpClient::Table msgHeaders;
  auto b_msg = message->Message();
  if (b_msg->HeaderTableIsSet()) {
    msgHeaders = b_msg->HeaderTable();
    uint64_t timestamp_rcvd = std::chrono::duration_cast<std::chrono::microseconds>(
                                  std::chrono::system_clock::now().time_since_epoch())
                                  .count();
    auto in_consumer_tag = message->ConsumerTag();
    std::string data;
    data = "{ \"spanname\": \"";
    data += in_consumer_tag;

    data += "\", \"timestamp_send\": \"";
    auto aux = msgHeaders.find("timestamp_send");
    if (aux != msgHeaders.end()) {
    data += std::to_string(msgHeaders.find("timestamp_send")->second.GetUint64());
    }

    data += "\", \"timestamp_rcvd\": \"";
    data += std::to_string(timestamp_rcvd);

    data += "\", \"x-b3-flags\": \"";
    auto aux = msgHeaders.find("x-b3-flags");
    if (aux != msgHeaders.end()) {
      data += aux->second.GetString();
    }

    data += "\", \"x-b3-parentspanid\": \"";
    aux = msgHeaders.find("x-b3-parentspanid");
    if (aux != msgHeaders.end()) {
      data += aux->second.GetString();
    }

    data += "\", \"x-b3-sampled\": \"";
    aux = msgHeaders.find("x-b3-sampled");
    if (aux != msgHeaders.end()) {
      data += aux->second.GetString();
    }

    data += "\", \"x-b3-spanid\": \"";
    aux = msgHeaders.find("x-b3-spanid");
    if (aux != msgHeaders.end()) {
      data += aux->second.GetString();
    }

    data += "\", \"x-b3-traceid\": \"";
    aux = msgHeaders.find("x-b3-traceid");
    if (aux != msgHeaders.end()) {
      data += aux->second.GetString();
    }

    data += "\" }";
    const char* dataToSend = data.c_str();
    udp::endpoint endpoint_(ip::address::from_string(commtrace_exporter_ip),commtrace_exporter_port);
    socket.send_to(boost::asio::buffer(dataToSend, strlen(dataToSend)), endpoint_);

    char reply[160];
    udp::endpoint sender;
    socket.receive_from(boost::asio::buffer(reply), sender);
    socket.close();

    std::string st_reply = boost::locale::conv::to_utf<char>(reply, "Latin1");
    st_reply = st_reply.substr(1, st_reply.size() - 2);
    boost::erase_all(st_reply, "\"");
    boost::erase_all(st_reply, " ");
    struct wc_dict {
      std::string name;
      std::string value;
    };
    std::vector<wc_dict> ctxt_fields;
    std::string token;
    wc_dict ctxt_field;
    char split_char = ',';
    std::istringstream split(st_reply);
    while (std::getline(split, token, split_char)) {
      size_t pos = token.find(':');
      ctxt_field.name = token.substr(0, pos);
      ctxt_field.value = token.substr(pos + 1);
      ctxt_fields.push_back(ctxt_field);
    }
    msgHeaders.clear();
    msgHeaders.insert(AmqpClient::TableEntry("x-b3-flags", ctxt_fields[0].value));
    msgHeaders.insert(AmqpClient::TableEntry("x-b3-parentspanid", ctxt_fields[1].value));
    msgHeaders.insert(AmqpClient::TableEntry("x-b3-sampled", ctxt_fields[2].value));
    msgHeaders.insert(AmqpClient::TableEntry("x-b3-spanid", ctxt_fields[3].value));
    msgHeaders.insert(AmqpClient::TableEntry("x-b3-traceid", ctxt_fields[4].value));
    b_msg->HeaderTable(msgHeaders);
  }
}

Message Channel::consume(std::string const& commtrace_exporter_ip, std::uint16_t const& commtrace_exporter_port) const {
  auto msg = _impl->BasicConsumeMessage();
  if((!(commtrace_exporter_ip.empty())) && (commtrace_exporter_port != 0)){
  create_tracer(msg,commtrace_exporter_ip,commtrace_exporter_port);
  }
  return from_internal_message(msg);
}

boost::optional<Message> Channel::consume_for(system_clock::duration const& duration,std::string const& commtrace_exporter_ip, std::uint16_t const& commtrace_exporter_port) const {
  auto msg = boost::shared_ptr<AmqpClient::Envelope>{};
  auto micros = duration_cast<microseconds>(duration).count();
  auto millis = (micros > 0) ? static_cast<int>(std::ceil(micros / 1000.0f)) : 0;
  if (_impl->BasicConsumeMessage(msg, millis)) {
    if (msg) {
      if((!(commtrace_exporter_ip.empty())) && (commtrace_exporter_port != 0)){
        create_tracer(msg,commtrace_exporter_ip,commtrace_exporter_port);
        }
      }
    return from_internal_message(msg);
  }
  return boost::none;
}

boost::optional<Message> Channel::consume_until(system_clock::time_point const& time_point,std::string const& commtrace_exporter_ip, std::uint16_t const& commtrace_exporter_port) const {
  if((!(commtrace_exporter_ip.empty())) && (commtrace_exporter_port != 0)){
    return consume_for(time_point - system_clock::now(),commtrace_exporter_ip,commtrace_exporter_port);
  }
  else{
    return consume_for(time_point - system_clock::now());
  }
}

std::vector<Message> Channel::consume_ready(std::string const& commtrace_exporter_ip, std::uint16_t const& commtrace_exporter_port) const {
  std::vector<is::Message> messages;
  for (;;) {
    auto maybe_msg = consume_for(milliseconds(0),commtrace_exporter_ip,commtrace_exporter_port);
    if (maybe_msg)
      messages.push_back(maybe_msg.get());
    else
      return messages;
  }
}

}  // namespace is