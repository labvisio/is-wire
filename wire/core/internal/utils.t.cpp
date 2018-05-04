
#include "gtest/gtest.h"
#include "utils.hpp"

namespace {

TEST(UtilsTest, PackUnpackEquality) {
  using namespace std::chrono;

  auto now = system_clock::time_point(seconds(129128));

  auto msg = is::Message{};
  auto cid = is::make_random_uid();
  msg.set_topic("topic")
      .set_correlation_id(cid)
      .set_reply_to("reply")
      .set_body("body")
      .set_content_type(is::wire::ContentType::PROTOBUF)
      .set_created_at(now)
      .set_deadline(now + milliseconds(6788));

  auto internal_msg = is::to_internal_message(msg);
  ASSERT_EQ(internal_msg->Body(), "body");
  ASSERT_EQ(internal_msg->CorrelationId(), std::to_string(cid));
  ASSERT_EQ(internal_msg->ContentType(), "application/x-protobuf");
  ASSERT_EQ(internal_msg->Timestamp(), 129128000);
  ASSERT_EQ(internal_msg->Expiration(), "6788");

  auto envelope = AmqpClient::Envelope::Create(internal_msg, "ctag", 666, "exch", false, "topic", 22);
  auto msg2 = is::from_internal_message(envelope);

  ASSERT_EQ(msg2.topic(), "topic");
  ASSERT_EQ(msg2.reply_to(), "reply");
  ASSERT_EQ(msg2.body(), msg.body());
  ASSERT_EQ(msg2.correlation_id(), cid);
  ASSERT_EQ(msg2.content_type(), is::wire::ContentType::PROTOBUF);
  ASSERT_EQ(msg2.created_at(), now);
  ASSERT_EQ(msg2.deadline(), now + milliseconds(6788));
  ASSERT_EQ(msg, msg2);
}

}  // namespace