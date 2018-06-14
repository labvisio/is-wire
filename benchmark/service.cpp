#include "../wire/core.hpp"
#include "../wire/rpc.hpp"
#include "is/msgs/image.pb.h"

using namespace is::vision;
using namespace is::wire;

Status hello(is::Context*, Image const&, Image*) {
  return is::make_status(is::wire::StatusCode::OK);
}

Status thrower(is::Context*, Image const&, Image*) {
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