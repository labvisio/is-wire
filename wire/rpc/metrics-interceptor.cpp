#include "metrics-interceptor.hpp"

namespace is {

MetricsInterceptor::MetricsInterceptor(std::shared_ptr<prometheus::Registry> const& reg)
    : registry(reg) {
  duration_family = &prometheus::BuildCounter()
                         .Name("duration_total")
                         .Help("Sum of request duration in milliseconds")
                         .Register(*registry);

  req_family = &prometheus::BuildCounter()
                    .Name("requests_total")
                    .Help("Number of requests processed")
                    .Register(*registry);

  ok_family = &prometheus::BuildCounter()
                   .Name("ok_total")
                   .Help("Number of OK responses")
                   .Register(*registry);

  de_family = &prometheus::BuildCounter()
                   .Name("de_total")
                   .Help("Number of DEADLINE_EXCEEDED responses")
                   .Register(*registry);
}

InterceptorConcept* MetricsInterceptor::copy() const {
  return new MetricsInterceptor(*this);
}

void MetricsInterceptor::before_call(Context*) {}

void MetricsInterceptor::after_call(Context* ctx) {
  auto duration = ctx->duration();
  auto topic = ctx->service();
  auto key_value = service_metrics.find(topic);

  if (key_value == service_metrics.end()) {
    ServiceMetrics metrics;
    metrics.duration = &duration_family->Add({{"service", topic}});
    metrics.req = &req_family->Add({{"service", topic}});
    metrics.ok = &ok_family->Add({{"service", topic}});
    metrics.de = &de_family->Add({{"service", topic}});
    auto iter_and_ok = service_metrics.emplace(topic, metrics);
    if (!iter_and_ok.second) {
      error("Failed to insert metric into map");
      return;
    }
    key_value = iter_and_ok.first;
  }

  auto metrics = key_value->second;
  metrics.duration->Increment(duration_cast<milliseconds>(duration).count());
  metrics.req->Increment();
  if (ctx->status().code() == wire::StatusCode::OK) {
    metrics.ok->Increment();
  } else if (ctx->status().code() == wire::StatusCode::DEADLINE_EXCEEDED) {
    metrics.de->Increment();
  }
}

}  // namespace is