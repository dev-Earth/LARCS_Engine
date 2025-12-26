#pragma once

#include "larcs/runtime/transport.hpp"
#include "larcs/runtime/zenoh_transport.hpp"

#include <google/protobuf/message.h>
#include <zenoh.h>

#include <functional>
#include <memory>
#include <string>

namespace larcs::runtime {

template <typename MessageT>
class Subscriber {
 public:
  using CallbackType = std::function<void(const MessageT&)>;

  Subscriber(std::shared_ptr<ZenohTransport> transport,
             const std::string& topic, CallbackType callback,
             QoSProfile qos = QoSProfile::Telemetry);
  ~Subscriber();

  // Delete copy constructor and assignment
  Subscriber(const Subscriber&) = delete;
  Subscriber& operator=(const Subscriber&) = delete;

  // Allow move
  Subscriber(Subscriber&&) noexcept = default;
  Subscriber& operator=(Subscriber&&) noexcept = default;

  const std::string& topic() const { return topic_; }

 private:
  static void zenoh_callback(z_loaned_sample_t* sample, void* context);

  std::shared_ptr<ZenohTransport> transport_;
  std::string topic_;
  CallbackType callback_;
  QoSProfile qos_;
  z_owned_subscriber_t subscriber_;
};

}  // namespace larcs::runtime
