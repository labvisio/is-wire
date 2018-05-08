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

void set_loglevel(std::shared_ptr<spdlog::logger> const& logger, loglevel level) {
  logger->set_level(level);
}

}