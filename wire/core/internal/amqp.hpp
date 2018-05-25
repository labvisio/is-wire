#pragma once

#include "../message.hpp"
#include "SimpleAmqpClient/SimpleAmqpClient.h"

namespace is {

uint64_t make_random_uid();
std::string hostname();
std::string consumer_id();

std::string content_type_to_string(wire::ContentType type);
wire::ContentType content_type_from_string(std::string const& str);

boost::shared_ptr<AmqpClient::BasicMessage> to_internal_message(Message const&);
Message from_internal_message(boost::shared_ptr<AmqpClient::Envelope> const&);

}  // namespace is