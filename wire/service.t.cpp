#include <chrono>
#include <thread>
#include "core.hpp"
#include "is/msgs/image.pb.h"
#include "rpc.hpp"
#include "rpc/log-interceptor.hpp"
#include "rpc/metrics-interceptor.hpp"
#include "zipkin/opentracing.h"

using namespace is::vision;
using namespace is::wire;

Status hello(is::Context* ctx, Image const& req, Image* rep) {
  std::this_thread::sleep_for(std::chrono::milliseconds(20));
  if (ctx->has_tracer()) { ctx->span()->SetTag("hodor", 10); }
  return is::make_status(is::wire::StatusCode::OK);
}

int main(int, char**) {
  auto tracer_options = zipkin::ZipkinOtTracerOptions{};
  tracer_options.service_name = "server";
  auto tracer = zipkin::makeZipkinOtTracer(tracer_options);

  auto channel = is::Channel{"amqp://localhost"};
  channel.set_tracer(tracer);

  auto provider = is::ServiceProvider{channel};

  is::LogInterceptor logs;
  provider.add_interceptor(logs);

  prometheus::Exposer exposer("8080");
  auto registry = std::make_shared<prometheus::Registry>();
  exposer.RegisterCollectable(registry);
  is::MetricsInterceptor metrics(registry);
  provider.add_interceptor(metrics);

  provider.delegate<Image, Image>("hodor.hello", hello);
  provider.run();
}