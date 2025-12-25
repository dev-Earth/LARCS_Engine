#include "larcs/runtime/subscriber.hpp"

#include <stdexcept>

namespace larcs::runtime {

// Implementation struct for Subscriber
template <typename MessageType>
struct Subscriber<MessageType>::Impl {
  std::string topic_name;
  CallbackType callback;

  Impl(std::string name, CallbackType cb) : topic_name(std::move(name)), callback(std::move(cb)) {
  }
};

template <typename MessageType>
Subscriber<MessageType>::Subscriber(const std::string& topic_name, CallbackType callback)
    : impl_(std::make_unique<Impl>(topic_name, std::move(callback))) {
}

template <typename MessageType>
Subscriber<MessageType>::~Subscriber() = default;

template <typename MessageType>
std::string Subscriber<MessageType>::GetTopicName() const {
  return impl_->topic_name;
}

}  // namespace larcs::runtime
