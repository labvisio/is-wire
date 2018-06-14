#include "context.hpp"

namespace is {

Context::Context(std::string const& service, Message const* req, Message* rep,
                 std::shared_ptr<opentracing::v1::Tracer> const& t)
    : _name(service), _req(req), _rep(rep), _tracer(t) {
  if (has_tracer()) {
    auto maybe_span_context = req->extract_tracing(_tracer);
    if (maybe_span_context) {
      _span_context = std::move(maybe_span_context.value());
      _span = _tracer->StartSpan(service, {opentracing::ChildOf(_span_context.get())});
    } else {
      _span = _tracer->StartSpan(service);
    }
  }
}

Context::~Context() {
  if (has_tracer()) { _span->Finish(); }
}

void Context::finish() {
  if (has_tracer()) { _rep->inject_tracing(_tracer, _span->context()); }
}

bool Context::has_tracer() const {
  return _tracer != nullptr;
}

std::unique_ptr<opentracing::v1::Span> Context::start_span(std::string const& name) const {
  if (has_tracer()) { return _tracer->StartSpan(name, {opentracing::ChildOf(&_span->context())}); }
  return nullptr;
}

opentracing::v1::Span* Context::span() const {
  return _span.get();
}

std::string Context::service() const {
  return _name;
}

system_clock::duration Context::duration() const {
  return system_clock::now() - _rep->created_at();
}

bool Context::deadline_exceeded() const {
  return _req->deadline_exceeded();
}

wire::Status Context::status() const {
  return _rep->status();
}

}  // namespace is