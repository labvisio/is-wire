#include "context.hpp"

namespace is {

Context::Context(std::string const& service, Message const* req, Message* rep,
                 std::shared_ptr<opentracing::v1::Tracer> const& t)
    : name(service), req(req), rep(rep), tracer(t) {
  if (tracer != nullptr) {
    auto reader = is::OtReader{req};
    auto maybe_span_context = tracer->Extract(reader);

    if (maybe_span_context) {
      span_context = std::move(maybe_span_context.value());
      span = tracer->StartSpan(service, {opentracing::ChildOf(span_context.get())});
    }
  }
}

Context::~Context() {
  if (tracer != nullptr) { span->Finish(); }
}

void Context::finish() {
  if (tracer != nullptr) {
    auto writer = is::OtWriter{rep};
    tracer->Inject(span->context(), writer);
  }
}

std::unique_ptr<opentracing::v1::Span> Context::start_span(std::string const& name) {
  if (tracer != nullptr) {
    return tracer->StartSpan(name, {opentracing::ChildOf(span_context.get())});
  }
  return nullptr;
}

std::string Context::service() const {
  return name;
}

system_clock::duration Context::duration() const {
  return system_clock::now() - req->created_at();
}

bool Context::deadline_exceeded() const {
  auto exceeded = req->deadline_exceeded();
  if (exceeded) { rep->set_status(wire::StatusCode::DEADLINE_EXCEEDED, ""); }
  return exceeded;
}

wire::Status Context::status() const {
  return rep->status();
}

}  // namespace is