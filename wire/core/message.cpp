#include "message.hpp"
#include "internal/amqp.hpp"
#include "subscription.hpp"

namespace is {

Message Message::create_reply(Message const& req) {
  auto rep = Message{};
  rep.set_topic(req.reply_to())
      .set_correlation_id(req.correlation_id())
      .set_content_type(req.content_type());
  return rep;
}

Message::Message()
    : _topic(),
      _sid(),
      _body(),
      _content_type(wire::ContentType::NONE),
      _created_at(system_clock::now()),
      _deadline(),
      _status(),
      _reply_to(),
      _correlation_id(0) {}

bool operator==(Message const& lhs, Message const& rhs) {
  return lhs._body == rhs._body && lhs._content_type == rhs._content_type &&
         lhs._created_at == rhs._created_at && lhs._deadline == rhs._deadline &&
         lhs._topic == rhs._topic && lhs._reply_to == rhs._reply_to &&
         lhs._correlation_id == rhs._correlation_id;
}

bool operator!=(Message const& lhs, Message const& rhs) {
  return !(lhs == rhs);
}

std::string Message::body() const {
  return _body;
}

std::string Message::topic() const {
  return _topic;
}

uint64_t Message::correlation_id() const {
  return _correlation_id;
}

std::string Message::reply_to() const {
  return _reply_to;
}

wire::ContentType Message::content_type() const {
  return _content_type;
}

system_clock::time_point Message::created_at() const {
  return _created_at;
}

system_clock::time_point Message::deadline() const {
  return _deadline;
}

std::string Message::subscription_id() const {
  return _sid;
}

wire::Status Message::status() const {
  return _status;
}

bool Message::has_body() const {
  return !_body.empty();
}

bool Message::has_topic() const {
  return !_topic.empty();
}

bool Message::has_correlation_id() const {
  return _correlation_id != 0;
}

bool Message::has_reply_to() const {
  return !_reply_to.empty();
}

bool Message::has_content_type() const {
  return _content_type != wire::ContentType::NONE;
}

bool Message::has_created_at() const {
  return _created_at.time_since_epoch().count() != 0;
}

bool Message::has_deadline() const {
  return _deadline.time_since_epoch().count() != 0;
}

bool Message::has_status() const {
  return !(_status.code() == wire::StatusCode::UNKNOWN && _status.why().empty());
}

Message& Message::set_body(std::string const& bytes) {
  _body = bytes;
  return *this;
}

Message& Message::set_topic(std::string const& topic) {
  _topic = topic;
  return *this;
}

Message& Message::set_correlation_id(uint64_t id) {
  _correlation_id = id;
  return *this;
}

Message& Message::set_reply_to(std::string const& reply_to) {
  _reply_to = reply_to;
  if (!has_correlation_id()) { set_correlation_id(make_random_uid()); }
  return *this;
}

Message& Message::set_reply_to(Subscription const& subscription) {
  set_reply_to(subscription.name());
  return *this;
}

Message& Message::set_content_type(wire::ContentType type) {
  _content_type = type;
  return *this;
}

Message& Message::set_created_at(system_clock::time_point const& time_point) {
  _created_at = time_point;
  return *this;
}

Message& Message::set_deadline(system_clock::time_point const& time_point) {
  _deadline = time_point;
  return *this;
}

Message& Message::set_deadline(system_clock::duration const& duration) {
  _deadline = system_clock::now() + duration;
  return *this;
}

Message& Message::set_subscription_id(std::string const& sid) {
  _sid = sid;
  return *this;
}

Message& Message::set_status(wire::Status const& status) {
  _status = status;
  return *this;
}

Message& Message::set_status(wire::StatusCode const& code, std::string const& why) {
  _status.set_code(code);
  _status.set_why(why);
  return *this;
}

}  // namespace is