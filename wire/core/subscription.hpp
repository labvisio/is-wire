#pragma once

#include <chrono>
#include <string>
#include <vector>
#include "boost/optional.hpp"

namespace AmqpClient {
class Channel;
}

namespace is {

class Message;
class Channel;

using namespace std::chrono;
class Subscription {
  boost::shared_ptr<AmqpClient::Channel> impl;
  std::string exchange;
  std::string queue;
  std::string tag;

 public:
  Subscription(Channel& channel, std::string const& name = "");
  Subscription(Subscription const&) = default;
  Subscription(Subscription&&) = default;

  std::string id() const;
  std::string name() const;

  void subscribe(std::string const& topic);
  void subscribe(std::vector<std::string> const& topics);
  void unsubscribe(std::string const& topic);
  void unsubscribe(std::vector<std::string> const& topics);
};

}  // namespace is