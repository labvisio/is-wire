
#include "opentracing.hpp"

namespace is {

OtWriter::OtWriter(Message* message) : msg(message) {}

opentracing::expected<void> OtWriter::Set(opentracing::string_view key,
                                          opentracing::string_view value) const {
  auto headers = msg->mutable_metadata();
  (*headers)[static_cast<std::string>(key)] = static_cast<std::string>(value);
  return {};
}

OtReader::OtReader(Message const* message) : msg(message) {}

opentracing::expected<void> OtReader::ForeachKey(F f) const {
  const auto starts_with_preffix = [](std::string const& str, std::string const& preffix) {
    return str.size() > preffix.size() && std::equal(preffix.begin(), preffix.end(), str.begin());
  };

  const auto preffix = std::string{"x-b3"};
  for (auto const& key_value : msg->metadata()) {
    if (starts_with_preffix(key_value.first, preffix)) {
      const auto was_successful = f(key_value.first, key_value.second);
      if (!was_successful) { return was_successful; }
    }
  }
  return {};
}  // namespace is

opentracing::expected<opentracing::string_view> OtReader::LookupKey(
    opentracing::string_view key) const {
  const auto headers = msg->metadata();
  const auto iter = headers.find(key);
  if (iter != headers.end()) {
    return opentracing::make_unexpected(opentracing::key_not_found_error);
  }
  return opentracing::string_view{iter->second};
}

}  // namespace is