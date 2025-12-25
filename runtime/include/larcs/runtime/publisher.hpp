#pragma once

#include <functional>
#include <memory>
#include <string>

namespace larcs::runtime {

// Publisher skeleton - to be extended with actual implementation
template <typename MessageType>
class Publisher {
 public:
  explicit Publisher(const std::string& topic_name);
  ~Publisher();

  // Delete copy constructor and assignment
  Publisher(const Publisher&) = delete;
  Publisher& operator=(const Publisher&) = delete;

  // Allow move
  Publisher(Publisher&&) noexcept = default;
  Publisher& operator=(Publisher&&) noexcept = default;

  // Publish a message
  void Publish(const MessageType& message);

  // Get topic name
  std::string GetTopicName() const;

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace larcs::runtime
