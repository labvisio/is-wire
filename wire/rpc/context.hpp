#pragma once

#include "message.hpp"

namespace is {

using namespace std::chrono;

class Context {
  const std::string name;
  Message const* req;
  Message* rep;
  system_clock::time_point start;
  system_clock::time_point end;

 public:
  Context(std::string const& service, Message const* request, Message* reply);
  Context(Context const&) = default;
  Context(Context&&) = default;

  void set_end(system_clock::time_point const& = system_clock::now());

  std::string service() const;
  system_clock::duration duration() const;
  is::Message const* request() const;
  is::Message* reply() const;
};

}  // namespace is