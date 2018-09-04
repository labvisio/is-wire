#pragma once

#include "../core/message.hpp"
#include "../core/opentracing.hpp"
#include "opentracing/span.h"

namespace is {

using namespace std::chrono;

class Context {
  const std::string _name;
  Message const* _req;
  Message* _rep;
  std::shared_ptr<opentracing::v1::Tracer> _tracer;
  std::unique_ptr<opentracing::v1::Span> _span;
  std::unique_ptr<opentracing::v1::SpanContext> _span_context;

 public:
  Context(std::string const& service, Message const* request, Message* reply,
          std::shared_ptr<opentracing::v1::Tracer> const&);
  Context(Context const&) = default;
  Context(Context&&) = default;
  ~Context();

  void finish();

  bool has_tracer() const;
  std::unique_ptr<opentracing::v1::Span> start_span(std::string const& name) const;
  opentracing::v1::Span* span() const;

  std::string service() const;
  bool deadline_exceeded() const;
  system_clock::duration duration() const;
  Status status() const;
};

}  // namespace is