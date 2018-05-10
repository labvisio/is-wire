#pragma once

#include "interceptor.hpp"
#include "../core/logger.hpp"

namespace is {

class LogInterceptor : public InterceptorConcept {
 public:
  LogInterceptor() = default;
  LogInterceptor(LogInterceptor const&) = default;
  LogInterceptor(LogInterceptor&&) = default;
  InterceptorConcept* copy() const;
  void before_call(Context* context);
  void after_call(Context* context);
};  // LogInterceptor

}  // namespace is