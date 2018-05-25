
#include "amqp.hpp"
#include "gtest/gtest.h"

namespace {

TEST(AmqpTest, PackUnpackEquality) {
  using namespace std::chrono;

  auto now = system_clock::time_point(seconds(129128));

  auto msg = is::Message{};
  auto cid = 0xd9f7be288fdad87f;
  msg.set_topic("topic")
      .set_correlation_id(cid)
      .set_reply_to("reply")
      .set_body("body")
      .set_content_type(is::wire::ContentType::PROTOBUF)
      .set_created_at(now)
      .set_status(is::wire::StatusCode::OK, "ok then")
      .set_deadline(now + milliseconds(6788));

  auto metadata = msg.mutable_metadata();
  (*metadata)["x-b3-flags"] = "1";
  (*metadata)["x-b3-parentspanid"] = "0000000000000000";
  (*metadata)["x-b3-sampled"] = "1";
  (*metadata)["x-b3-spanid"] = "d9f7be288fdad87f";
  (*metadata)["x-b3-traceid"] = "d19f9083bcc4f48a";

  auto internal_msg = is::to_internal_message(msg);
  ASSERT_EQ(internal_msg->Body(), "body");
  ASSERT_EQ(internal_msg->CorrelationId(), "D9F7BE288FDAD87F");
  ASSERT_EQ(internal_msg->ContentType(), "application/x-protobuf");
  ASSERT_EQ(internal_msg->Timestamp(), 129128000);
  ASSERT_EQ(internal_msg->Expiration(), "6788");

  auto envelope =
      AmqpClient::Envelope::Create(internal_msg, "ctag", 666, "exch", false, "topic", 22);
  auto msg2 = is::from_internal_message(envelope);

  ASSERT_EQ(msg2.topic(), "topic");
  ASSERT_EQ(msg2.reply_to(), "reply");
  ASSERT_EQ(msg2.body(), msg.body());
  ASSERT_EQ(msg2.correlation_id(), cid);
  ASSERT_EQ(msg2.content_type(), is::wire::ContentType::PROTOBUF);
  ASSERT_EQ(msg2.created_at(), now);
  ASSERT_EQ(msg2.status().code(), is::wire::StatusCode::OK);
  ASSERT_EQ(msg2.status().why(), "ok then");
  ASSERT_EQ(msg2.deadline(), now + milliseconds(6788));
  ASSERT_EQ(msg2.metadata(), msg.metadata());
  ASSERT_EQ(msg, msg2);

  // Should be able to remove '-' from ids
  internal_msg->CorrelationId("d9f7----be2-88fdad87f");
  auto msg3 = is::from_internal_message(envelope);
  ASSERT_EQ(msg3.correlation_id(), cid);

  // Invalid ids should be safely ignored
  internal_msg->CorrelationId("d9f7----bMZXMNZMNXMZNXe2-88fdad87f");
  auto msg4 = is::from_internal_message(envelope);
}

}  // namespace