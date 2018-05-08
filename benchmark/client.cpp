#include <chrono>
#include "is/msgs/image.pb.h"
#include "is/msgs/utils.hpp"
#include "is/wire/core.hpp"

using namespace google::protobuf;

#include "zipkin/opentracing.h"

int main(int, char**) {
  using namespace std::chrono;

  auto channel = is::Channel{"amqp://localhost"};
  auto subscription = is::Subscription{channel};

  auto object = is::vision::Image{};
  auto request = is::Message{object};
  request.set_reply_to(subscription);

  {
    auto before = system_clock::now();

    for (int i = 1; i < 10e3; ++i) {
      request.set_correlation_id(i);
      channel.publish("hodor.hello", request);
      auto reply = channel.consume();
      assert(request.correlation_id() == reply.correlation_id() &&
             reply.status().code() == is::wire::StatusCode::OK);
    }

    is::info("hello: {} msgs/s  [{}B]",
             1e6 / (duration_cast<microseconds>(system_clock::now() - before).count() / 10e3), request.body().size());
  }
  {
    auto before = system_clock::now();

    for (int i = 1; i < 10e3; ++i) {
      request.set_correlation_id(i);
      channel.publish("hodor.thrower", request);
      auto reply = channel.consume();
      assert(request.correlation_id() == reply.correlation_id() &&
             reply.status().code() == is::wire::StatusCode::INTERNAL_ERROR);
    }

    is::info("thrower: {} msgs/s [{}B]",
             1e6 / (duration_cast<microseconds>(system_clock::now() - before).count() / 10e3), request.body().size());
  }
}