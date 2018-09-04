
#include "status.hpp"

namespace is {

Status::Status(StatusCode code, std::string const& why) : _code(code), _why(why) {}
Status::Status(wire::Status const& wire_status) : Status(wire_status.code(), wire_status.why()) {}

void Status::set_code(StatusCode code) {
  _code = code;
}

StatusCode Status::code() const {
  return _code;
}

bool Status::ok() const {
  return _code == StatusCode::OK;
}

void Status::set_why(std::string const& why) {
  _why = why;
}

std::string const& Status::why() const {
  return _why;
}

Status make_status(StatusCode code, std::string const& why) {
  return Status{code, why};
}

}  // namespace is
