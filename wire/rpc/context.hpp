#pragma once

#include "message.hpp"
#include "opentracing.hpp"
#include "opentracing/span.h"

namespace is {

using namespace std::chrono;

class Context {
  const std::string name;
  Message const* req;
  Message* rep;
  std::shared_ptr<opentracing::v1::Tracer> tracer;
  std::unique_ptr<opentracing::v1::Span> span;
  std::unique_ptr<opentracing::v1::SpanContext> span_context;

 public:
  Context(std::string const& service, Message const* request, Message* reply,
          std::shared_ptr<opentracing::v1::Tracer> const&);
  Context(Context const&) = default;
  Context(Context&&) = default;
  ~Context();

  void finish();
  std::unique_ptr<opentracing::v1::Span> start_span(std::string const& name);

  std::string service() const;
  bool deadline_exceeded() const;
  system_clock::duration duration() const;
  wire::Status status() const;
};

}  // namespace is