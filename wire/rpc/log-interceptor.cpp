#include "log-interceptor.hpp"

namespace is {

InterceptorConcept* LogInterceptor::copy() const {
  return new LogInterceptor(*this);
}

void LogInterceptor::before_call(Context*) {}

void LogInterceptor::after_call(Context* ctx) {
  auto service = ctx->service();
  auto took = duration_cast<milliseconds>(ctx->duration()).count();
  auto code = ctx->status().code();
  auto status = StatusCode_Name(code);
  auto why = ctx->status().why();

  switch (code) {
  case StatusCode::OK: is::info("service={} took={}ms code={}", service, took, status); break;
  case StatusCode::INTERNAL_ERROR:
    error("service={} took={}ms code={} why='{}'", service, took, status, why);
    break;
  default: warn("service={} took={}ms code={} why='{}'", service, took, status, why);
  }
}

}  // namespace is