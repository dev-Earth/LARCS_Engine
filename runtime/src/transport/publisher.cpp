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
    z_internal_publisher_null(&publisher_);
    return;
  }

  // Create key expression for the topic
  // Remove leading slash if present (Zenoh 1.7.1 doesn't allow leading slashes)
  std::string topic_key = topic_;
  if (!topic_key.empty() && topic_key[0] == '/') {
    topic_key = topic_key.substr(1);
  }
  
  z_owned_keyexpr_t keyexpr;
  z_result_t keyexpr_result = z_keyexpr_from_str_autocanonize(&keyexpr, topic_key.c_str());
  if (keyexpr_result != Z_OK) {
    spdlog::error("Publisher: failed to create keyexpr for topic: {} (error code: {})", 
                  topic_, static_cast<int>(keyexpr_result));
    z_internal_publisher_null(&publisher_);
    return;
  }

  // Configure publisher options based on QoS
  z_publisher_options_t options;
  z_publisher_options_default(&options);

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
  z_result_t result = z_declare_publisher(z_loan(transport_->session()), &publisher_,
                                          z_loan(keyexpr), &options);

  z_drop(z_move(keyexpr));

  if (result != Z_OK) {
    spdlog::error("Publisher: failed to declare publisher for topic: {}",
                  topic_);
    z_internal_publisher_null(&publisher_);
  } else {
    spdlog::debug("Publisher created for topic: {}", topic_);
  }
}

template <typename MessageT>
Publisher<MessageT>::~Publisher() {
  if (z_internal_publisher_check(&publisher_)) {
    z_undeclare_publisher(z_move(publisher_));
    spdlog::debug("Publisher destroyed for topic: {}", topic_);
  }
}

template <typename MessageT>
bool Publisher<MessageT>::publish(const MessageT& msg) {
  if (!z_internal_publisher_check(&publisher_)) {
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

  // Create bytes from serialized data
  z_owned_bytes_t payload;
  if (z_bytes_copy_from_buf(&payload, 
                            (const uint8_t*)serialized.data(),
                            serialized.size()) != Z_OK) {
    spdlog::error("Publisher: failed to create payload for topic: {}", topic_);
    return false;
  }

  // Publish data
  z_publisher_put_options_t put_options;
  z_publisher_put_options_default(&put_options);
  int result = z_publisher_put(z_loan(publisher_),
                                z_move(payload), &put_options);

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
#include "geometry.pb.h"
template class larcs::runtime::Publisher<larcs::msgs::Twist>;
template class larcs::runtime::Publisher<larcs::msgs::Pose>;
template class larcs::runtime::Publisher<larcs::msgs::Vector3>;
