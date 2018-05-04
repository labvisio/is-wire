#pragma once

#include <chrono>
#include <string>
#include "boost/optional.hpp"
#include "boost/shared_ptr.hpp"

namespace AmqpClient {
class Channel;
}

namespace is {

using namespace std::chrono;

class Subscription;
class Message;

class Channel {
  boost::shared_ptr<AmqpClient::Channel> impl;
  std::string exchange;
  friend boost::shared_ptr<AmqpClient::Channel> get_impl(Subscription&, Channel& channel);
  friend std::string get_exchange(Subscription&, Channel const& channel);

 public:
  Channel(std::string const& uri);
  Channel(Channel const&) = default;
  Channel(Channel&&) = default;

  void publish(std::string const& topic, Message const& message) const;
  void publish(Message const& message) const;

  Message consume() const;
  boost::optional<Message> consume_for(system_clock::duration const& duration) const;
  boost::optional<Message> consume_until(system_clock::time_point const& time_point) const;
};

}  // namespace is