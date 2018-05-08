#include "service-provider.hpp"

namespace is {

ServiceProvider::ServiceProvider(Channel const& c) : channel(c) {}

void ServiceProvider::add_interceptor(Interceptor const& interceptor) {
  interceptors.push_back(interceptor);
}

bool ServiceProvider::serve(Message const& in) const {
  const auto service = services.find(in.subscription_id());
  const auto found = service != services.end();
  if (found) {
    auto out = Message::create_reply(in);
    auto ctx = Context{service->first, &in, &out};

    for (auto&& interceptor : interceptors) before_call(interceptor, &ctx);
    service->second(&ctx, in, &out);
    ctx.set_end();
    for (auto&& interceptor : interceptors) after_call(interceptor, &ctx);
    channel.publish(out);
  }
  return found;
}

void ServiceProvider::run() const {
  for (;;) { serve(channel.consume()); }
}

}  // namespace is