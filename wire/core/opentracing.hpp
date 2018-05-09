
#pragma once

#include "message.hpp"
#include "opentracing/propagation.h"
#include "opentracing/tracer.h"

namespace is {

class OtWriter : public opentracing::TextMapWriter {
 public:
  OtWriter(Message*);

  opentracing::expected<void> Set(opentracing::string_view key,
                                  opentracing::string_view value) const override;

 private:
  Message* msg;
};

class OtReader : public opentracing::TextMapReader {
 public:
  OtReader(Message const*);

  using F = std::function<opentracing::expected<void>(opentracing::string_view,
                                                      opentracing::string_view)>;

  opentracing::expected<void> ForeachKey(F f) const override;

  opentracing::expected<opentracing::string_view> LookupKey(
      opentracing::string_view key) const override;

 private:
  Message const* msg;
};

}  // namespace is