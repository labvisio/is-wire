#include "is/msgs/image.pb.h"
#include "is/wire/core.hpp"
#include "is/wire/rpc.hpp"

using namespace is::vision;
using namespace is::wire;

Status hello(is::Context* ctx, Image const& req, Image* rep) {
  return is::make_status(is::wire::StatusCode::OK);
}

Status thrower(is::Context* ctx, Image const& req, Image* rep) {
  throw std::runtime_error("big evil exception");
  return is::make_status(is::wire::StatusCode::OK);
}

int main(int, char**) {
  auto channel = is::Channel{"amqp://localhost"};
  auto provider = is::ServiceProvider{channel};
  provider.delegate<Image, Image>("hodor.hello", hello);
  provider.delegate<Image, Image>("hodor.thrower", thrower);
  provider.run();
}