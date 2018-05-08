#include "is/wire/rpc.hpp"
#include "is/msgs/image.pb.h"
#include "is/wire/rpc/log-interceptor.hpp"

using namespace is::vision;
using namespace is::wire;

Status hello(is::Context* ctx, Image const& req, Image* rep) {
  return is::make_status(is::wire::StatusCode::OK);
}

Status thrower(is::Context* ctx, Image const& req, Image* rep) {
  throw std::runtime_error("hodor");
  return is::make_status(is::wire::StatusCode::OK);
}

int main(int, char**) {
  auto channel = is::Channel{"amqp://localhost"};
  auto provider = is::ServiceProvider{channel};
  //is::LogInterceptor logs;
  //provider.add_interceptor(logs);
  provider.delegate<Image, Image>("hodor.hello", hello);
  provider.delegate<Image, Image>("hodor.thrower", thrower);
  provider.run();
}