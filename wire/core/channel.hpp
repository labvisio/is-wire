#pragma once

#include <chrono>
#include <string>
#include <vector>
#include "boost/optional.hpp"
#include "boost/shared_ptr.hpp"
#include "opentracing/tracer.h"

namespace AmqpClient {
class Channel;
}

namespace is {

using namespace std::chrono;

class Subscription;
class Message;

class Channel {
  boost::shared_ptr<AmqpClient::Channel> _impl;
  std::string _exchange;
  std::shared_ptr<opentracing::v1::Tracer> _tracer;

  friend boost::shared_ptr<AmqpClient::Channel> get_impl(Subscription&, Channel& channel);
  friend std::string get_exchange(Subscription&, Channel const& channel);

 public:
  Channel(std::string const& uri);
  Channel(Channel const&) = default;
  Channel(Channel&&) = default;

  void set_tracer(std::shared_ptr<opentracing::v1::Tracer> const&);
  std::shared_ptr<opentracing::v1::Tracer> tracer() const;

  void publish(std::string const& topic, Message const& message) const;
  void publish(Message const& message) const;

  Message consume(std::string const& commtrace_exporter_ip = std::string(), std::uint16_t const& commtrace_exporter_port = 0) const;
  boost::optional<Message> consume_for(system_clock::duration const& duration,std::string const& commtrace_exporter_ip = std::string(), std::uint16_t const& commtrace_exporter_port = 0) const;
  boost::optional<Message> consume_until(system_clock::time_point const& time_point,std::string const& commtrace_exporter_ip = std::string(), std::uint16_t const& commtrace_exporter_port = 0) const;
  std::vector<Message> consume_ready(std::string const& commtrace_exporter_ip = std::string(), std::uint16_t const& commtrace_exporter_port = 0) const;
};

}  // namespace is
