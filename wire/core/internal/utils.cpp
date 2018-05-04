
#include "utils.hpp"

namespace is {

boost::shared_ptr<AmqpClient::BasicMessage> to_internal_message(is::Message const& message) {
  using namespace std::chrono;

  auto internal = AmqpClient::BasicMessage::Create(message.body());

  if (message.has_content_type()) {
    internal->ContentType(content_type_to_string(message.content_type()));
  }

  if (message.has_created_at()) {
    internal->Timestamp(
        duration_cast<milliseconds>(message.created_at().time_since_epoch()).count());

    if (message.has_deadline()) {
      internal->Expiration(std::to_string(
          duration_cast<milliseconds>(message.deadline() - message.created_at()).count()));
    }
  }

  if (message.has_reply_to()) { internal->ReplyTo(message.reply_to()); }
  if (message.has_correlation_id()) {
    internal->CorrelationId(std::to_string(message.correlation_id()));
  }

  if (message.has_status()) {
    std::string bytes;
    google::protobuf::util::MessageToJsonString(message.status(), &bytes);
    AmqpClient::Table headers;
    headers[AmqpClient::TableKey("rpc-status")] = AmqpClient::TableValue(bytes);
    internal->HeaderTable(headers);
  }

  return internal;
}

is::Message from_internal_message(boost::shared_ptr<AmqpClient::Envelope> const& internal) {
  using namespace std::chrono;

  auto message = is::Message{};
  message.set_subscription_id(internal->ConsumerTag());
  message.set_body(internal->Message()->Body());

  if (internal->Message()->ContentTypeIsSet()) {
    message.set_content_type(content_type_from_string(internal->Message()->ContentType()));
  }

  if (internal->Message()->TimestampIsSet()) {
    message.set_created_at(
        system_clock::time_point(milliseconds(internal->Message()->Timestamp())));

    if (internal->Message()->ExpirationIsSet()) {
      message.set_deadline(message.created_at() +
                           milliseconds(std::stoi(internal->Message()->Expiration())));
    }
  }

  message.set_topic(internal->RoutingKey());

  if (internal->Message()->ReplyToIsSet()) { message.set_reply_to(internal->Message()->ReplyTo()); }
  if (internal->Message()->CorrelationIdIsSet()) {
    message.set_correlation_id(std::stoull(internal->Message()->CorrelationId()));
  }

  if (internal->Message()->HeaderTableIsSet()) {
    // TODO: (Breaking Change) Change to two different headers to avoid protobuf and/or json parser
    // dependency
    auto headers = internal->Message()->HeaderTable();
    auto keyval_it = headers.find("rpc-status");
    if (keyval_it == headers.end()) {
      wire::Status status;
      google::protobuf::util::JsonStringToMessage(keyval_it->second.GetString(), &status);
      message.set_status(status);
    }
  }

  return message;
}

std::string content_type_to_string(wire::ContentType type) {
  switch (type) {
  case wire::ContentType::NONE: return "";
  case wire::ContentType::JSON: return "application/json";
  case wire::ContentType::PROTOBUF: return "application/x-protobuf";
  }
}

wire::ContentType content_type_from_string(std::string const& str) {
  if (str == "application/x-protobuf") return wire::ContentType::PROTOBUF;
  if (str == "application/json") return wire::ContentType::JSON;
  return wire::ContentType::NONE;
}

uint64_t make_random_uid() {
  static std::mt19937_64 engine(std::random_device{}());
  static std::uniform_int_distribution<uint64_t> distribution(1,
                                                              std::numeric_limits<uint64_t>::max());
  return distribution(engine);
}

std::string hostname() {
  return boost::asio::ip::host_name();
}

std::string consumer_id() {
  std::stringstream ss;
  ss << hostname() << '/' << std::uppercase << std::setfill('0') << std::setw(4) << std::hex
     << make_random_uid();
  return ss.str();
}

}  // namespace is