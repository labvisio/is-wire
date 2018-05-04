#include "service-provider.hpp"

namespace is {

ServiceProvider::ServiceProvider(Channel const& c) : channel(c) {}

bool ServiceProvider::serve(Message const& in) const {
  const auto service = services.find(in.subscription_id());
  is::info("{}", in.correlation_id());
  if (service != services.end()) {
    auto out = service->second(in);
    channel.publish(out);
  }

  return service != services.end();
}

void ServiceProvider::run() const {
  for (;;) {
    serve(channel.consume());
  }
}

}  // namespace is