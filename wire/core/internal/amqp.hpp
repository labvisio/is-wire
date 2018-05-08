#pragma once

#include <boost/asio.hpp>
#include <chrono>
#include <iomanip>
#include <limits>
#include "../message.hpp"
#include "SimpleAmqpClient/SimpleAmqpClient.h"
#include "amqp_ot_carrier.hpp"
#include "opentracing/tracer.h"

namespace is {

uint64_t make_random_uid();
std::string hostname();
std::string consumer_id();

std::string content_type_to_string(wire::ContentType type);
wire::ContentType content_type_from_string(std::string const& str);

boost::shared_ptr<AmqpClient::BasicMessage> to_internal_message(
    Message const&, std::shared_ptr<opentracing::v1::Tracer> const&);

Message from_internal_message(boost::shared_ptr<AmqpClient::Envelope> const&,
                              std::shared_ptr<opentracing::v1::Tracer> const&);

}  // namespace is