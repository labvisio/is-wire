#pragma once

#include <chrono>
#include <unordered_map>
#include "boost/optional.hpp"
#include "content_type.hpp"
#include "google/protobuf/message.h"
#include "google/protobuf/util/json_util.h"
#include "opentracing/propagation.h"
#include "status.hpp"

namespace is {

using namespace std::chrono;

class Subscription;

class Message {
  std::string _topic;
  std::string _sid;

  std::string _body;
  ContentType _content_type;
  system_clock::time_point _created_at;
  system_clock::time_point _deadline;

  Status _status;
  std::string _reply_to;
  uint64_t _correlation_id;

  std::unordered_map<std::string, std::string> _metadata;

 public:
  Message();
  template <typename T>
  Message(T const& object);
  Message(Message const&) = default;
  Message(Message&&) = default;
  Message& operator=(Message const&) = default;

  friend bool operator==(Message const& lhs, Message const& rhs);
  friend bool operator!=(Message const& lhs, Message const& rhs);

  bool has_body() const;
  std::string body() const;
  // Sets the raw content of the message (rarely used). Users should prefer using the pack function
  Message& set_body(std::string const& bytes);

  bool has_topic() const;
  std::string topic() const;
  Message& set_topic(std::string const& topic);

  bool has_correlation_id() const;
  uint64_t correlation_id() const;
  Message& set_correlation_id(uint64_t id);

  bool has_reply_to() const;
  std::string reply_to() const;
  Message& set_reply_to(std::string const& subscription_name);
  Message& set_reply_to(Subscription const&);

  bool has_content_type() const;
  ContentType content_type() const;
  Message& set_content_type(ContentType);

  bool has_created_at() const;
  system_clock::time_point created_at() const;
  Message& set_created_at(system_clock::time_point const&);

  bool has_deadline() const;
  system_clock::time_point deadline() const;
  bool deadline_exceeded() const;
  // Duration from now where the message is still valid
  Message& set_deadline(system_clock::duration const&);
  Message& set_deadline(system_clock::time_point const&);

  std::string subscription_id() const;
  Message& set_subscription_id(std::string const& subscription_id);

  bool has_status() const;
  Status status() const;
  Message& set_status(Status const& status);
  Message& set_status(StatusCode const&, std::string const& why = "");

  std::unordered_map<std::string, std::string> const& metadata() const;
  std::unordered_map<std::string, std::string>* mutable_metadata();

  void inject_tracing(std::shared_ptr<opentracing::v1::Tracer> const&,
                      opentracing::v1::SpanContext const&);
  opentracing::expected<std::unique_ptr<opentracing::SpanContext>> extract_tracing(
      std::shared_ptr<opentracing::v1::Tracer> const&) const;

  template <typename T>
  boost::optional<T> unpack() const;

  template <typename T>
  Message& pack(T const& object);

  static Message create_reply(Message const& request);
};

template <typename T>
Message::Message(T const& object) : Message() {
  pack(object);
}

template <typename T>
boost::optional<T> Message::unpack() const {
  boost::optional<T> optional;
  T object;
  if (content_type() == ContentType::NONE || content_type() == ContentType::PROTOBUF) {
    if (object.ParseFromString(_body)) optional = object;
  } else if (content_type() == ContentType::JSON) {
    if (google::protobuf::util::JsonStringToMessage(_body, &object).ok()) optional = object;
  } else if (content_type() == ContentType::PROTOTEXT) {
    // TODO
  }
  return optional;
}

template <typename T>
Message& Message::pack(T const& object) {
  if (content_type() == ContentType::NONE) {
    // User didn't provide a valid type, default to protobuf.
    set_content_type(ContentType::PROTOBUF);
  }

  if (content_type() == ContentType::PROTOBUF) {
    object.SerializeToString(&_body);
  } else if (content_type() == ContentType::JSON) {
    google::protobuf::util::JsonPrintOptions options;
    options.always_print_primitive_fields = true;
    google::protobuf::util::MessageToJsonString(object, &_body, options);
  } else if (content_type() == ContentType::PROTOTEXT) {
    // TODO
  }
  return *this;
}

}  // namespace is
