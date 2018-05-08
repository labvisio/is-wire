
#pragma once

#include "SimpleAmqpClient/SimpleAmqpClient.h"
#include "opentracing/propagation.h"
#include "opentracing/tracer.h"

namespace is {

class AmqpOtCarrier : public opentracing::TextMapReader, public opentracing::TextMapWriter {
 public:
  AmqpOtCarrier(AmqpClient::Table& table);

  opentracing::expected<void> Set(opentracing::string_view key,
                                  opentracing::string_view value) const override;

  using F = std::function<opentracing::expected<void>(opentracing::string_view,
                                                      opentracing::string_view)>;

  opentracing::expected<void> ForeachKey(F f) const override;

  opentracing::expected<opentracing::string_view> LookupKey(
      opentracing::string_view key) const override;

 private:
  AmqpClient::Table& headers;
};

}  // namespace is