#include "internal/utils.hpp"
//
#include "channel.hpp"
#include "subscription.hpp"

namespace is {

Subscription::Subscription(Channel& channel, std::string const& q) {
  impl = get_impl(*this, channel);
  exchange = get_exchange(*this, channel);
  tag = consumer_id();
  queue = q.empty() ? tag : q;
  impl->DeclareQueue(queue, /*passive=*/false, /*durable=*/false, /*exclusive=*/false,
                     /*auto_delete=*/true);
  impl->BasicConsume(queue, tag, /*nolocal=*/false, /*noack=*/true,
                     /*exclusive=*/false);
  subscribe(queue); // to receive replies
}

std::string Subscription::name() const {
  return queue;
}

std::string Subscription::id() const {
  return tag;
}

void Subscription::subscribe(std::string const& topic) {
  impl->BindQueue(queue, exchange, topic);
}

void Subscription::subscribe(std::vector<std::string> const& topics) {
  for (auto&& topic : topics)
    subscribe(topic);
}

void Subscription::unsubscribe(std::string const& topic) {
  impl->UnbindQueue(queue, exchange, topic);
}

void Subscription::unsubscribe(std::vector<std::string> const& topics) {
  for (auto&& topic : topics)
    unsubscribe(topic);
}

}  // namespace is