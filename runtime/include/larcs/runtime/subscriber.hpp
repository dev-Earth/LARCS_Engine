#pragma once

#include <functional>
#include <memory>
#include <string>

namespace larcs::runtime {

// Subscriber skeleton - to be extended with actual implementation
template <typename MessageType>
class Subscriber {
 public:
  using CallbackType = std::function<void(const MessageType&)>;

  Subscriber(const std::string& topic_name, CallbackType callback);
  ~Subscriber();

  // Delete copy constructor and assignment
  Subscriber(const Subscriber&) = delete;
  Subscriber& operator=(const Subscriber&) = delete;

  // Allow move
  Subscriber(Subscriber&&) noexcept = default;
  Subscriber& operator=(Subscriber&&) noexcept = default;

  // Get topic name
  std::string GetTopicName() const;

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace larcs::runtime
