#pragma once

#include "spdlog/fmt/ostr.h"
#include "spdlog/spdlog.h"

namespace is {

std::shared_ptr<spdlog::logger> logger();
int set_loglevel(char level);

template <class... Args>
inline void info(Args&&... args) {
  logger()->info(args...);
}

template <class... Args>
inline void warn(Args&&... args) {
  logger()->warn(args...);
}

template <class... Args>
inline void error(Args&&... args) {
  logger()->error(args...);
}

template <class... Args>
inline void critical(Args&&... args) {
  logger()->critical(args...);
  std::exit(-1);
}

}  // namespace is