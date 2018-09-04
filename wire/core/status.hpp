#pragma once

#include "wire.pb.h"

namespace is {

using StatusCode = wire::StatusCode;

class Status {
  StatusCode _code;
  std::string _why;

 public:
  Status(StatusCode code = StatusCode::UNKNOWN, std::string const& why = "");
  Status(wire::Status const& wire_status);
  operator wire::Status() const {
    auto status = wire::Status{};
    status.set_code(code());
    status.set_why(why());
    return status;
  }

  void set_code(StatusCode code);
  StatusCode code() const;
  bool ok() const;

  void set_why(std::string const& why);
  std::string const& why() const;
};

Status make_status(StatusCode code = StatusCode::OK, std::string const& why = "");

}  // namespace is