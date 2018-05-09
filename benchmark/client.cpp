#include <benchmark/benchmark.h>
#include <chrono>
#include "is/msgs/image.pb.h"
#include "is/msgs/utils.hpp"
#include "is/wire/core.hpp"

using namespace google::protobuf;

static void BM_MessagingOverhead(benchmark::State& state) {
  auto channel = is::Channel{"amqp://localhost"};
  auto subscription = is::Subscription{channel};
  for (auto _ : state) {
    auto object = is::vision::Image{};
    std::string bytes(state.range(0), 'x');
    object.set_data(bytes);

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
    auto object = is::vision::Image{};
    std::string bytes(state.range(0), 'x');
    object.set_data(bytes);

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
