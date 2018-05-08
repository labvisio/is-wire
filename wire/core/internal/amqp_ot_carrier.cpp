
#include "amqp_ot_carrier.hpp"

namespace is {

AmqpOtCarrier::AmqpOtCarrier(AmqpClient::Table& table) : headers(table) {}
opentracing::expected<void> AmqpOtCarrier::Set(opentracing::string_view key,
                                               opentracing::string_view value) const {
  headers[static_cast<std::string>(key)] = static_cast<std::string>(value);
  return {};
}

opentracing::expected<void> AmqpOtCarrier::ForeachKey(F f) const {
  for (auto& key_value : headers) {
    if (key_value.first[0] == 'x') {
      auto was_successful = f(key_value.first, key_value.second.GetString());
      if (!was_successful) { return was_successful; }
    }
  }
  return {};
}

opentracing::expected<opentracing::string_view> AmqpOtCarrier::LookupKey(
    opentracing::string_view key) const {
  auto iter = headers.find(key);
  if (iter != headers.end()) {
    return opentracing::make_unexpected(opentracing::key_not_found_error);
  }
  return opentracing::string_view{iter->second.GetString()};
}

}  // namespace is