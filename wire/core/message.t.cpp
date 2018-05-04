#include "google/protobuf/struct.pb.h"
#include "google/protobuf/util/message_differencer.h"
#include "gtest/gtest.h"
#include "internal/utils.hpp"
#include "message.hpp"

namespace {

google::protobuf::Struct make_test_object() {
  auto object = google::protobuf::Struct{};
  auto fields = object.mutable_fields();
  auto value = google::protobuf::Value{};
  value.set_string_value("my str value");
  (*fields)["field"] = value;
  return object;
}

TEST(MessageTest, MemberFunctions) {
  using namespace std::chrono;

  auto message = is::Message{};
  ASSERT_TRUE(message.has_created_at());

  auto topic = "game.of.chairs";
  ASSERT_FALSE(message.has_topic());
  message.set_topic(topic);
  ASSERT_TRUE(message.has_topic());
  ASSERT_EQ(message.topic(), topic);

  auto ctype = is::wire::ContentType::JSON;
  ASSERT_FALSE(message.has_content_type());
  message.set_content_type(ctype);
  ASSERT_TRUE(message.has_content_type());
  ASSERT_EQ(message.content_type(), ctype);

  auto cid = is::make_random_uid();
  ASSERT_FALSE(message.has_correlation_id());
  message.set_correlation_id(cid);
  ASSERT_TRUE(message.has_correlation_id());
  ASSERT_EQ(message.correlation_id(), cid);

  auto deadline = system_clock::time_point(seconds(66666666)) + milliseconds(387);
  ASSERT_FALSE(message.has_deadline());
  message.set_deadline(deadline);
  ASSERT_TRUE(message.has_deadline());
  ASSERT_EQ(message.deadline(), deadline);

  auto reply_to = "john.snow";
  ASSERT_FALSE(message.has_reply_to());
  message.set_reply_to(reply_to);
  ASSERT_TRUE(message.has_reply_to());
  ASSERT_EQ(message.reply_to(), reply_to);

  ASSERT_FALSE(message.has_body());
  message.pack(make_test_object());
  ASSERT_TRUE(message.has_body());

  message.set_status(is::wire::StatusCode::ALREADY_EXISTS);
  ASSERT_EQ(message.status().code(), is::wire::StatusCode::ALREADY_EXISTS);
  ASSERT_EQ(message.status().why(), "");

  is::wire::Status status;
  status.set_code(is::wire::StatusCode::OK);
  status.set_why("roger that");
  message.set_status(status);
  ASSERT_EQ(message.status().code(), is::wire::StatusCode::OK);
  ASSERT_EQ(message.status().why(), "roger that");

  message.set_subscription_id("mysid");
  ASSERT_EQ(message.subscription_id(), "mysid");
}

TEST(MessageTest, FastConstruction) {
  auto object = make_test_object();
  auto message = is::Message{object};
  ASSERT_TRUE(message.has_content_type());
  ASSERT_TRUE(message.has_body());
  ASSERT_TRUE(message.has_created_at());

  message.set_reply_to("john.snow");
  ASSERT_TRUE(message.has_correlation_id());
  ASSERT_TRUE(message.has_reply_to());

  // Should give the same object
  auto maybe_unpacked_object = message.unpack<google::protobuf::Struct>();
  ASSERT_NE(maybe_unpacked_object, boost::none);

  auto unpacked_object = *maybe_unpacked_object;
  ASSERT_TRUE(google::protobuf::util::MessageDifferencer::Equals(unpacked_object, object));
}

}  // namespace