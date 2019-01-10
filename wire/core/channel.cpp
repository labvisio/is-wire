#include "channel.hpp"
#include <cmath>
#include "internal/amqp.hpp"
#include "logger.hpp"

namespace is {

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
  auto msg = to_internal_message(message);
  _impl->BasicPublish(_exchange, topic, msg);
}

void Channel::publish(Message const& message) const {
  publish(message.topic(), message);
}

Message Channel::consume() const {
  auto msg = _impl->BasicConsumeMessage();
  return from_internal_message(msg);
}

boost::optional<Message> Channel::consume_for(system_clock::duration const& duration) const {
  auto msg = boost::shared_ptr<AmqpClient::Envelope>{};
  auto micros = duration_cast<microseconds>(duration).count();
  auto millis = (micros > 0) ? static_cast<int>(std::ceil(micros / 1000.0f)) : 0;
  if (_impl->BasicConsumeMessage(msg, millis)) { return from_internal_message(msg); }
  return boost::none;
}

boost::optional<Message> Channel::consume_until(system_clock::time_point const& time_point) const {
  return consume_for(time_point - system_clock::now());
}

std::vector<Message> Channel::consume_ready() const {
  std::vector<is::Message> messages;
  for (;;) {
    auto maybe_msg = consume_for(milliseconds(0));
    if (maybe_msg)
      messages.push_back(maybe_msg.get());
    else
      return messages;
  }
}

}  // namespace is