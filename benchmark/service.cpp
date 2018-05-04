#include "google/protobuf/struct.pb.h"
#include "is/wire/rpc.hpp"

using namespace google::protobuf;

is::wire::Status hello(is::Context* ctx, Struct const& req, Struct* rep) {
  auto fields = rep->mutable_fields();
  auto value = google::protobuf::Value{};
  value.set_string_value("valeu em, parabens");
  (*fields)["field"] = value;

  return is::make_status(is::wire::StatusCode::OK);
}

int main(int, char**) {
  auto channel = is::Channel{"amqp://localhost"};
  auto provider = is::ServiceProvider{channel};
  provider.delegate<Struct, Struct>("hodor.hello", hello);
  provider.run();
}