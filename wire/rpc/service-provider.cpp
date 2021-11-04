#include "service-provider.hpp"

namespace is {

ServiceProvider::ServiceProvider(Channel const& c) : channel(c) {}

void ServiceProvider::add_interceptor(Interceptor const& interceptor) {
  interceptors.push_back(interceptor);
}

bool ServiceProvider::serve(Message const& req) const {
  const auto sid = req.subscription_id();
  const auto service = services.find(sid);
  const auto name = names.find(sid);

  const auto found = service != services.end() && name != names.end();
  if (found) {
    auto rep = Message::create_reply(req);
    auto ctx = Context{name->second, &req, &rep, channel.tracer()};

    for (auto&& interceptor : interceptors) before_call(interceptor, &ctx);

    if (!ctx.deadline_exceeded()) { service->second(&ctx, req, &rep); }
    // We check for timeout here instead of checking before publishing to avoid Status inconsistency
    // beetwen what the interceptors see and what actually happens
    auto timeouted = ctx.deadline_exceeded();
    if (timeouted) { rep.set_status(StatusCode::DEADLINE_EXCEEDED); }
    
    for (auto&& interceptor : interceptors) after_call(interceptor, &ctx);
    ctx.finish();

    if (!timeouted) { channel.publish(rep); }
  }
  return found;
}

void ServiceProvider::run(std::string const& commtrace_exporter_ip = std::string(), std::uint16_t const& commtrace_exporter_port = 0) const {
  if((!(commtrace_exporter_ip.empty())) && (commtrace_exporter_port != 0)){
  for (;;) { serve(channel.consume(commtrace_exporter_ip,commtrace_exporter_port)); }
  }
  else{
  for (;;) { serve(channel.consume()); }
  }
}

}  // namespace is