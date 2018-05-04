#include "internal/utils.hpp"
//
#include "channel.hpp"
#include "logger.hpp"

namespace is {

boost::shared_ptr<AmqpClient::Channel> get_impl(Subscription&, Channel& channel) {
  return channel.impl;
}

std::string get_exchange(Subscription&, Channel const& channel) {
  return channel.exchange;
}

Channel::Channel(std::string const& uri) {
  impl = AmqpClient::Channel::CreateFromUri(uri);
  exchange = "is";
}

void Channel::publish(std::string const& topic, Message const& message) const {
  auto msg = to_internal_message(message);
  impl->BasicPublish(exchange, topic, msg);
}

void Channel::publish(Message const& message) const {
  publish(message.topic(), message);
}

Message Channel::consume() const {
  auto msg = impl->BasicConsumeMessage();
  return from_internal_message(msg);
}

boost::optional<Message> Channel::consume_for(system_clock::duration const& duration) const {
  auto msg = boost::shared_ptr<AmqpClient::Envelope>{};
  auto millis = duration_cast<milliseconds>(duration).count();
  if (millis >= 0 && impl->BasicConsumeMessage(msg, millis)) { return from_internal_message(msg); }
  return boost::none;
}

boost::optional<Message> Channel::consume_until(system_clock::time_point const& time_point) const {
  return consume_for(time_point - system_clock::now());
}

}  // namespace is