#include <chrono>
#include "google/protobuf/struct.pb.h"
#include "is/wire/core.hpp"

using namespace google::protobuf;

int main(int, char**) {
  using namespace std::chrono;

  auto channel = is::Channel{"amqp://localhost"};
  auto subscription = is::Subscription{channel};

  auto object = Struct{};
  auto fields = object.mutable_fields();
  auto value = Value{};
  value.set_string_value("my str value");
  (*fields)["field"] = value;

  auto request = is::Message{object};
  request.set_reply_to(subscription);

  auto before = system_clock::now();
  for (int i = 0; i < 10e3; ++i) {
    request.set_correlation_id(i);
    channel.publish("hodor.hello", request);
    auto reply = channel.consume();
    //is::info("{} {}", request.correlation_id(), reply.correlation_id());
    if (request.correlation_id() == reply.correlation_id()) {
      auto object = reply.unpack<Struct>();
      //object->PrintDebugString();
    }
  }
  is::info("{} us/msg", duration_cast<microseconds>(system_clock::now() - before).count()/10e3);
}