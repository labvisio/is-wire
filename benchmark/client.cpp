#include <benchmark/benchmark.h>
#include <chrono>
#include "google/protobuf/struct.pb.h"
#include "../wire/core.hpp"

using namespace google::protobuf;

static void BM_MessagingOverhead(benchmark::State& state) {
  auto channel = is::Channel{"amqp://localhost"};
  auto subscription = is::Subscription{channel};
  for (auto _ : state) {
    auto object = google::protobuf::Struct{};
    std::string bytes(state.range(0), 'x');
    auto value = (*object.mutable_fields())["data"];
    value.set_string_value(bytes);

    auto request = is::Message{object};
    request.set_reply_to(subscription);

    channel.publish("hodor.hello", request);

    auto reply = channel.consume();
    assert(request.correlation_id() == reply.correlation_id() &&
           reply.status().code() == is::wire::StatusCode::OK);
  }
  state.counters["Rate"] = benchmark::Counter(state.iterations(), benchmark::Counter::kIsRate);
}

BENCHMARK(BM_MessagingOverhead)->RangeMultiplier(8)->Range(8, 8 << 10);

static void BM_MessagingOverheadWithException(benchmark::State& state) {
  auto channel = is::Channel{"amqp://localhost"};
  auto subscription = is::Subscription{channel};
  for (auto _ : state) {
    auto object = google::protobuf::Struct{};
    std::string bytes(state.range(0), 'x');
    auto value = (*object.mutable_fields())["data"];
    value.set_string_value(bytes);

    auto request = is::Message{object};
    request.set_reply_to(subscription);

    channel.publish("hodor.thrower", request);

    auto reply = channel.consume();
    assert(request.correlation_id() == reply.correlation_id() &&
           reply.status().code() == is::wire::StatusCode::INTERNAL_ERROR);
  }
  state.counters["Rate"] = benchmark::Counter(state.iterations(), benchmark::Counter::kIsRate);
}

BENCHMARK(BM_MessagingOverheadWithException)->RangeMultiplier(8)->Range(8, 8 << 10);
BENCHMARK_MAIN();
