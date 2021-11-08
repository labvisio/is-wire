#pragma once

#include <exception>
#include <functional>
#include <unordered_map>
#include "../core/channel.hpp"
#include "../core/logger.hpp"
#include "../core/message.hpp"
#include "../core/subscription.hpp"
#include "context.hpp"
#include "interceptor.hpp"

namespace is {

class ServiceProvider {
  Channel channel;
  std::unordered_map<std::string, std::function<void(Context*, Message const&, Message*)>> services;
  std::unordered_map<std::string, std::string> names;
  std::vector<Interceptor> interceptors;

 public:
  ServiceProvider(Channel const&);
  ServiceProvider(ServiceProvider const&) = default;
  ServiceProvider(ServiceProvider&&) = default;

  // Setup interceptor to customize the behaviour of this class
  void add_interceptor(Interceptor const&);

  // Bind a function to a particular topic, so everytime a message is received in this topic the
  // function will be called
  template <typename Request, typename Reply>
  void delegate(std::string const& service,
                std::function<Status(Context*, Request, Reply*)> const& handler);

  // Attempts to serve message, returning false on failure.
  bool serve(Message const& request) const;

  // Blocks the current thread listening for requests
  void run(std::string const& commtrace_exporter_ip = std::string(), std::uint16_t const& commtrace_exporter_port = 0) const;
};

template <typename Request, typename Reply>
void ServiceProvider::delegate(
    std::string const& service,
    std::function<Status(Context*, Request, Reply*)> const& service_impl) {
  Subscription subscription{channel, service};
  services[subscription.id()] = [=](Context* ctx, Message const& in, Message* out) {
    auto request = in.unpack<Request>();
    if (request) {
      try {
        auto reply = Reply{};
        out->set_status(service_impl(ctx, *request, &reply));
        out->pack(reply);
      } catch (std::exception const& e) {
        out->set_status(
            StatusCode::INTERNAL_ERROR,
            fmt::format("Service '{}' throwed an exception\n: '{}'", service, e.what()));
      } catch (...) {
        out->set_status(StatusCode::INTERNAL_ERROR,
                        fmt::format("Service '{}' throwed unkown exception of type '{}'", service,
                                    typeid(std::current_exception()).name()));
      }
    } else {
      out->set_status(StatusCode::FAILED_PRECONDITION,
                      fmt::format("Expected request type '{}' but received something else",
                                  Request{}.GetMetadata().descriptor->full_name()));
    }
    return out;
  };
  names[subscription.id()] = service;
}

}  // namespace is