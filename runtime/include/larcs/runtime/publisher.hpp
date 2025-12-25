#pragma once

#include "larcs/runtime/transport.hpp"
#include "larcs/runtime/zenoh_transport.hpp"

#include <google/protobuf/message.h>
#include <zenoh.h>

#include <memory>
#include <string>

namespace larcs::runtime {

template <typename MessageT>
class Publisher {
 public:
  Publisher(std::shared_ptr<ZenohTransport> transport,
            const std::string& topic, QoSProfile qos = QoSProfile::Telemetry);
  ~Publisher();

  // Delete copy constructor and assignment
  Publisher(const Publisher&) = delete;
  Publisher& operator=(const Publisher&) = delete;

  // Allow move
  Publisher(Publisher&&) noexcept = default;
  Publisher& operator=(Publisher&&) noexcept = default;

  bool publish(const MessageT& msg);
  const std::string& topic() const { return topic_; }

 private:
  std::shared_ptr<ZenohTransport> transport_;
  std::string topic_;
  QoSProfile qos_;
  z_owned_publisher_t publisher_;
};

}  // namespace larcs::runtime
