#include "larcs/runtime/publisher.hpp"

#include <stdexcept>

namespace larcs::runtime {

// Implementation struct for Publisher
template <typename MessageType>
struct Publisher<MessageType>::Impl {
  std::string topic_name;

  explicit Impl(std::string name) : topic_name(std::move(name)) {
  }
};

template <typename MessageType>
Publisher<MessageType>::Publisher(const std::string& topic_name)
    : impl_(std::make_unique<Impl>(topic_name)) {
}

template <typename MessageType>
Publisher<MessageType>::~Publisher() = default;

template <typename MessageType>
void Publisher<MessageType>::Publish(const MessageType& message) {
  // Skeleton implementation - to be extended
  // For now, this is a no-op
  (void)message;  // Suppress unused parameter warning
}

template <typename MessageType>
std::string Publisher<MessageType>::GetTopicName() const {
  return impl_->topic_name;
}

}  // namespace larcs::runtime
