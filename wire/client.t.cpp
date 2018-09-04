#include <chrono>
#include "core.hpp"
#include "google/protobuf/struct.pb.h"
#include "zipkin/opentracing.h"

int main(int, char**) {
  using namespace std::chrono;

  auto tracer_options = zipkin::ZipkinOtTracerOptions{};
  tracer_options.service_name = "client";
  auto tracer = zipkin::makeZipkinOtTracer(tracer_options);

  auto channel = is::Channel{"amqp://localhost"};
  auto subscription = is::Subscription{channel};

  for (int i = 0; i < 20; ++i) {
    auto span = tracer->StartSpan("req");

    auto object = google::protobuf::Struct{};
    auto request = is::Message{object};
    request.set_reply_to(subscription)
        .set_deadline(milliseconds(25))
        .inject_tracing(tracer, span->context());

    channel.publish("hodor.hello", request);

    auto reply = channel.consume_until(request.deadline());
    if (reply && reply->status().ok()) {
      if (request.correlation_id() == reply->correlation_id()) { is::info("{}", i); }
    } else {
      is::warn("deadline exceeded");
    }

    span->Finish();
  }
  tracer->Close();
}