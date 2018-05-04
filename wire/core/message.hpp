#pragma once

#include <chrono>
#include "boost/optional.hpp"
#include "google/protobuf/message.h"
#include "google/protobuf/util/json_util.h"
#include "is/msgs/wire.pb.h"

namespace is {

using namespace std::chrono;

class Subscription;

class Message {
  std::string _topic;
  std::string _sid;

  std::string _body;
  wire::ContentType _content_type;
  system_clock::time_point _created_at;
  system_clock::time_point _deadline;

  wire::Status _status;
  std::string _reply_to;
  uint64_t _correlation_id;

 public:
  Message();
  template <typename T>
  Message(T const& object);
  Message(Message const&) = default;
  Message(Message&&) = default;

  friend bool operator==(Message const& lhs, Message const& rhs);
  friend bool operator!=(Message const& lhs, Message const& rhs);

  std::string body() const;
  std::string topic() const;
  uint64_t correlation_id() const;
  std::string reply_to() const;
  wire::ContentType content_type() const;
  system_clock::time_point created_at() const;
  system_clock::time_point deadline() const;
  std::string subscription_id() const;
  wire::Status status() const;

  bool has_body() const;
  bool has_topic() const;
  bool has_correlation_id() const;
  bool has_reply_to() const;
  bool has_content_type() const;
  bool has_created_at() const;
  bool has_deadline() const;
  bool has_status() const;

  // Sets the raw content of the message (rarely used). Users should prefer using the pack function
  Message& set_body(std::string const& bytes);
  Message& set_topic(std::string const& topic);
  Message& set_correlation_id(uint64_t id);
  Message& set_reply_to(std::string const& subscription_name);
  Message& set_reply_to(Subscription const&);
  Message& set_content_type(wire::ContentType);
  Message& set_created_at(system_clock::time_point const&);
  Message& set_deadline(system_clock::time_point const&);
  // Duration from now where the message is still valid
  Message& set_deadline(system_clock::duration const&);
  Message& set_subscription_id(std::string const& subscription_id);
  Message& set_status(wire::Status const& status);
  Message& set_status(wire::StatusCode const&, std::string const& why = "");

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
  T object;
  if (_content_type == wire::ContentType::PROTOBUF) {
    if (object.ParseFromString(_body)) return object;
  }
  if (_content_type == wire::ContentType::JSON) {
    if (google::protobuf::util::JsonStringToMessage(_body, &object).ok()) return object;
  }

  // User didn't provide a valid type, try to guess.
  if (google::protobuf::util::JsonStringToMessage(_body, &object).ok()) {
    // set_content_type(ContentType::JSON);
    return object;
  }
  if (object.ParseFromString(_body)) {
    // set_content_type(ContentType::PROTOBUF);
    return object;
  }

  return boost::none;
}

template <typename T>
Message& Message::pack(T const& object) {
  switch (_content_type) {
  case wire::ContentType::NONE:
    // User didn't provide a valid type, default to protobuf.
    set_content_type(wire::ContentType::PROTOBUF);
    // [[fallthrough]]
  case wire::ContentType::PROTOBUF: object.SerializeToString(&_body); break;
  case wire::ContentType::JSON: {
    google::protobuf::util::JsonPrintOptions options;
    options.always_print_primitive_fields = true;
    google::protobuf::util::MessageToJsonString(object, &_body, options);
  }
  }
  return *this;
}

}  // namespace is