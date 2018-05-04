#include "logger.hpp"

namespace is {

std::shared_ptr<spdlog::logger> logger() {
  static auto ptr = [] {
    auto ptr = spdlog::stdout_color_mt("is");
    ptr->set_pattern("[%L][%t][%d-%m-%Y %H:%M:%S:%e] %v");
    return ptr;
  }();
  return ptr;
}

int set_loglevel(char level) {
  if (level == 'i')
    spdlog::set_level(spdlog::level::info);
  else if (level == 'w')
    spdlog::set_level(spdlog::level::warn);
  else if (level == 'e')
    spdlog::set_level(spdlog::level::err);
  else
    return -1;  // failed
  return 0;     // success
}

}