#include "larcs/runtime/publisher.hpp"

#include "larcs/runtime/logger.hpp"

#include <spdlog/spdlog.h>

namespace larcs::runtime {

template <typename MessageT>
Publisher<MessageT>::Publisher(std::shared_ptr<ZenohTransport> transport,
                               const std::string& topic, QoSProfile qos)
    : transport_(transport), topic_(topic), qos_(qos) {
  if (!transport_ || !transport_->is_running()) {
    spdlog::error("Publisher: transport not initialized for topic: {}", topic_);
    publisher_ = z_publisher_null();
    return;
  }

  // Create key expression for the topic
  z_owned_keyexpr_t keyexpr = z_keyexpr_new(topic_.c_str());
  if (!z_check(keyexpr)) {
    spdlog::error("Publisher: failed to create keyexpr for topic: {}", topic_);
    publisher_ = z_publisher_null();
    return;
  }

  // Configure publisher options based on QoS
  z_publisher_options_t options = z_publisher_options_default();

  switch (qos_) {
    case QoSProfile::Control:
      // High reliability, low latency
      options.congestion_control = Z_CONGESTION_CONTROL_BLOCK;
      options.priority = Z_PRIORITY_REAL_TIME;
      break;
    case QoSProfile::Telemetry:
      // Latest value priority
      options.congestion_control = Z_CONGESTION_CONTROL_DROP;
      options.priority = Z_PRIORITY_DATA;
      break;
    case QoSProfile::Perception:
      // Best effort, large data
      options.congestion_control = Z_CONGESTION_CONTROL_DROP;
      options.priority = Z_PRIORITY_DATA_LOW;
      break;
  }

  // Create publisher
  publisher_ = z_declare_publisher(z_loan(transport_->session()),
                                   z_loan(keyexpr), &options);

  z_drop(z_move(keyexpr));

  if (!z_check(publisher_)) {
    spdlog::error("Publisher: failed to declare publisher for topic: {}",
                  topic_);
  } else {
    spdlog::debug("Publisher created for topic: {}", topic_);
  }
}

template <typename MessageT>
Publisher<MessageT>::~Publisher() {
  if (z_check(publisher_)) {
    z_undeclare_publisher(z_move(publisher_));
    spdlog::debug("Publisher destroyed for topic: {}", topic_);
  }
}

template <typename MessageT>
bool Publisher<MessageT>::publish(const MessageT& msg) {
  if (!z_check(publisher_)) {
    spdlog::error("Publisher: not initialized for topic: {}", topic_);
    return false;
  }

  // Serialize protobuf message
  std::string serialized;
  if (!msg.SerializeToString(&serialized)) {
    spdlog::error("Publisher: failed to serialize message for topic: {}",
                  topic_);
    return false;
  }

  // Publish data
  z_publisher_put_options_t put_options = z_publisher_put_options_default();
  int result = z_publisher_put(z_loan(publisher_),
                                (const uint8_t*)serialized.data(),
                                serialized.size(), &put_options);

  if (result < 0) {
    spdlog::error("Publisher: failed to publish to topic: {}", topic_);
    return false;
  }

  spdlog::trace("Published {} bytes to topic: {}", serialized.size(), topic_);
  return true;
}

}  // namespace larcs::runtime

// Explicit template instantiation for common types
// This is needed because the implementation is in a .cpp file
#include "larcs/msgs/geometry.pb.h"
template class larcs::runtime::Publisher<larcs::msgs::Twist>;
template class larcs::runtime::Publisher<larcs::msgs::Pose>;
template class larcs::runtime::Publisher<larcs::msgs::Vector3>;
