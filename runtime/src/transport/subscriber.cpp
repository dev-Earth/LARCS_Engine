#include "larcs/runtime/subscriber.hpp"

#include "larcs/runtime/logger.hpp"

#include <spdlog/spdlog.h>

namespace larcs::runtime {

// Context structure to pass callback information to Zenoh
template <typename MessageT>
struct SubscriberContext {
  std::function<void(const MessageT&)> callback;
  std::string topic;
};

template <typename MessageT>
Subscriber<MessageT>::Subscriber(std::shared_ptr<ZenohTransport> transport,
                                 const std::string& topic,
                                 CallbackType callback, QoSProfile qos)
    : transport_(transport), topic_(topic), callback_(callback), qos_(qos) {
  if (!transport_ || !transport_->is_running()) {
    spdlog::error("Subscriber: transport not initialized for topic: {}",
                  topic_);
    subscriber_ = z_subscriber_null();
    return;
  }

  // Create key expression for the topic
  z_owned_keyexpr_t keyexpr = z_keyexpr_new(topic_.c_str());
  if (!z_check(keyexpr)) {
    spdlog::error("Subscriber: failed to create keyexpr for topic: {}", topic_);
    subscriber_ = z_subscriber_null();
    return;
  }

  // Create context for the callback
  auto* context = new SubscriberContext<MessageT>();
  context->callback = callback_;
  context->topic = topic_;

  // Create closure for Zenoh callback with proper cleanup
  auto drop_callback = [](void* ctx) {
    delete static_cast<SubscriberContext<MessageT>*>(ctx);
  };
  z_owned_closure_sample_t closure = z_closure(zenoh_callback, drop_callback, context);

  // Declare subscriber
  subscriber_ = z_declare_subscriber(z_loan(transport_->session()),
                                     z_loan(keyexpr), z_move(closure), nullptr);

  z_drop(z_move(keyexpr));

  if (!z_check(subscriber_)) {
    spdlog::error("Subscriber: failed to declare subscriber for topic: {}",
                  topic_);
    delete context;
  } else {
    spdlog::debug("Subscriber created for topic: {}", topic_);
  }
}

template <typename MessageT>
Subscriber<MessageT>::~Subscriber() {
  if (z_check(subscriber_)) {
    z_undeclare_subscriber(z_move(subscriber_));
    spdlog::debug("Subscriber destroyed for topic: {}", topic_);
  }
}

template <typename MessageT>
void Subscriber<MessageT>::zenoh_callback(const z_sample_t* sample,
                                          void* context) {
  auto* sub_context = static_cast<SubscriberContext<MessageT>*>(context);

  // Extract payload
  z_owned_slice_t payload = z_sample_payload(sample);
  const uint8_t* data = z_slice_data(z_loan(payload));
  size_t size = z_slice_len(z_loan(payload));

  // Deserialize protobuf message
  MessageT msg;
  if (!msg.ParseFromArray(data, size)) {
    spdlog::error("Subscriber: failed to parse message for topic: {}",
                  sub_context->topic);
    z_drop(z_move(payload));
    return;
  }

  z_drop(z_move(payload));

  // Call user callback
  try {
    sub_context->callback(msg);
  } catch (const std::exception& e) {
    spdlog::error("Subscriber: exception in callback for topic {}: {}",
                  sub_context->topic, e.what());
  }
}

}  // namespace larcs::runtime

// Explicit template instantiation for common types
#include "larcs/msgs/geometry.pb.h"
template class larcs::runtime::Subscriber<larcs::msgs::Twist>;
template class larcs::runtime::Subscriber<larcs::msgs::Pose>;
template class larcs::runtime::Subscriber<larcs::msgs::Vector3>;
