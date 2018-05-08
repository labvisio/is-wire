#include "context.hpp"

namespace is {

Context::Context(std::string const& service, Message const* req, Message* rep)
    : name(service), req(req), rep(rep), start(system_clock::now()) {}

void Context::set_end(system_clock::time_point const& time_point) {
  end = time_point;
}

std::string Context::service() const {
  return name;
}

system_clock::duration Context::duration() const {
  return end - start;
}

is::Message const* Context::request() const {
  return req;
}

is::Message* Context::reply() const {
  return rep;
}

}  // namespace is