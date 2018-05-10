#include "service-provider.hpp"

namespace is {

ServiceProvider::ServiceProvider(Channel const& c) : channel(c) {}

void ServiceProvider::add_interceptor(Interceptor const& interceptor) {
  interceptors.push_back(interceptor);
}

bool ServiceProvider::serve(Message const& in) const {
  const auto sid = in.subscription_id();
  const auto service = services.find(sid);
  const auto name = names.find(sid);

  const auto found = service != services.end() && name != names.end();
  if (found) {
    auto out = Message::create_reply(in);
    auto ctx = Context{name->second, &in, &out, channel.tracer()};

    for (auto&& interceptor : interceptors) before_call(interceptor, &ctx);
    if (!ctx.deadline_exceeded()) { service->second(&ctx, in, &out); }
    for (auto&& interceptor : interceptors) after_call(interceptor, &ctx);
    ctx.finish();

    channel.publish(out);
  }
  return found;
}

void ServiceProvider::run() const {
  for (;;) { serve(channel.consume()); }
}

}  // namespace is