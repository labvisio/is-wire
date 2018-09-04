#include "../wire/core.hpp"
#include "../wire/rpc.hpp"
#include "google/protobuf/struct.pb.h"

is::Status hello(is::Context*, google::protobuf::Struct const&, google::protobuf::Struct*) {
  return is::make_status(is::wire::StatusCode::OK);
}

is::Status thrower(is::Context*, google::protobuf::Struct const&, google::protobuf::Struct*) {
  throw std::runtime_error("big evil exception");
  return is::make_status(is::wire::StatusCode::OK);
}

int main(int, char**) {
  auto channel = is::Channel{"amqp://localhost"};
  auto provider = is::ServiceProvider{channel};
  provider.delegate<google::protobuf::Struct, google::protobuf::Struct>("hodor.hello", hello);
  provider.delegate<google::protobuf::Struct, google::protobuf::Struct>("hodor.thrower", thrower);
  provider.run();
}