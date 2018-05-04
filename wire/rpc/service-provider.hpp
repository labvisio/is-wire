#pragma once

#include <exception>
#include <functional>
#include <unordered_map>
#include "channel.hpp"
#include "context.hpp"
#include "is/msgs/utils.hpp"
#include "is/msgs/wire.pb.h"
#include "logger.hpp"
#include "message.hpp"
#include "subscription.hpp"

namespace is {

class ServiceProvider {
  Channel channel;
  std::unordered_map<std::string, std::function<Message(Message const&)>> services;
  Context context;

 public:
  ServiceProvider(Channel const&);
  ServiceProvider(ServiceProvider const&) = default;
  ServiceProvider(ServiceProvider&&) = default;

  // Setup interceptor to customize the behaviour of this class
  // void add_interceptor();

  // Bind a function to a particular topic, so everytime a message is received in this topic the
  // function will be called
  template <typename Request, typename Reply>
  void delegate(std::string const& service,
                std::function<wire::Status(Context*, Request, Reply*)> const& handler);

  //
  bool serve(Message const& request) const;

  // Blocks the current thread listening for requests
  void run() const;
};

template <typename Request, typename Reply>
void ServiceProvider::delegate(
    std::string const& service,
    std::function<wire::Status(Context*, Request, Reply*)> const& service_impl) {
  Subscription subscription{channel, service};
  services[subscription.id()] = [=](Message const& in) -> Message {
    auto out = Message::create_reply(in);

    auto request = in.unpack<Request>();
    if (request) {
      try {
        auto reply = Reply{};
        out.set_status(service_impl(&this->context, *request, &reply));
        out.pack(reply);
      } catch (std::exception const& e) {
        out.set_status(wire::StatusCode::INTERNAL_ERROR,
                       fmt::format("Service '{}' throwed an exception\n: '{}'", service, e.what()));
      } catch (...) {
        out.set_status(wire::StatusCode::INTERNAL_ERROR,
                       fmt::format("Service '{}' throwed unkown exception of type '{}'", service,
                                   typeid(std::current_exception()).name()));
      }
    } else {
      out.set_status(wire::StatusCode::FAILED_PRECONDITION,
                     fmt::format("Expected request type '{}' but received something else",
                                 Request{}.GetMetadata().descriptor->full_name()));
    }
    return out;
  };
}

}  // namespace is