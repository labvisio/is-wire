#include "log-interceptor.hpp"

namespace is {

InterceptorConcept* LogInterceptor::copy() const {
  return new LogInterceptor(*this);
}

void LogInterceptor::before_call(Context*) {}

void LogInterceptor::after_call(Context* ctx) {
  auto service = ctx->service();
  auto took = duration_cast<milliseconds>(ctx->duration());
  auto code = ctx->reply()->status().code();
  auto status = wire::StatusCode_Name(code);
  auto why = ctx->reply()->status().why();

  switch (code) {
  case wire::StatusCode::OK: info("{};{};{}", service, took, status); break;
  case wire::StatusCode::INTERNAL_ERROR: error("{};{};{};'{}'", service, took, status, why); break;
  default: warn("{};{};{};'{}'", service, took, status, why);
  }
}

}  // namespace is