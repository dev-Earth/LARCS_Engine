#pragma once
#include <mcap/writer.hpp>
#include <spdlog/spdlog.h>
#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <mutex>

namespace larcs::runtime {

class Recorder {
public:
  explicit Recorder(const std::string& filepath);
  ~Recorder();
  
  bool open();
  void close();
  bool is_open() const;
  
  // Add topic to recording
  template<typename MessageT>
  bool add_topic(const std::string& topic);
  
  // Record a message
  template<typename MessageT>
  bool record(const std::string& topic, const MessageT& msg, uint64_t timestamp_ns);
  
private:
  std::string filepath_;
  std::unique_ptr<mcap::McapWriter> writer_;
  std::unordered_map<std::string, mcap::ChannelId> channels_;
  std::mutex mutex_;
  bool open_ = false;
};

// Template method implementations
template<typename MessageT>
bool Recorder::add_topic(const std::string& topic) {
  std::lock_guard<std::mutex> lock(mutex_);
  
  if (!open_) {
    spdlog::error("Recorder not open");
    return false;
  }
  
  // Check if channel already exists
  if (channels_.find(topic) != channels_.end()) {
    return true;
  }
  
  // Get protobuf descriptor
  MessageT msg;
  const google::protobuf::Descriptor* descriptor = msg.GetDescriptor();
  const google::protobuf::FileDescriptor* file_descriptor = descriptor->file();
  
  // Create schema
  mcap::Schema schema(descriptor->full_name(), "protobuf", file_descriptor->DebugString());
  writer_->addSchema(schema);
  
  // Create channel
  mcap::Channel channel(topic, "protobuf", schema.id);
  writer_->addChannel(channel);
  
  channels_[topic] = channel.id;
  spdlog::debug("Added topic to recording: {}", topic);
  
  return true;
}

template<typename MessageT>
bool Recorder::record(const std::string& topic, const MessageT& msg, uint64_t timestamp_ns) {
  std::lock_guard<std::mutex> lock(mutex_);
  
  if (!open_) {
    spdlog::error("Recorder not open");
    return false;
  }
  
  auto it = channels_.find(topic);
  if (it == channels_.end()) {
    spdlog::error("Topic not added to recorder: {}", topic);
    return false;
  }
  
  // Serialize message
  std::string data;
  if (!msg.SerializeToString(&data)) {
    spdlog::error("Failed to serialize message for topic: {}", topic);
    return false;
  }
  
  // Create MCAP message
  mcap::Message mcap_msg;
  mcap_msg.channelId = it->second;
  mcap_msg.sequence = 0; // TODO: add sequence tracking
  mcap_msg.logTime = timestamp_ns;
  mcap_msg.publishTime = timestamp_ns;
  mcap_msg.dataSize = data.size();
  mcap_msg.data = reinterpret_cast<const std::byte*>(data.data());
  
  auto status = writer_->write(mcap_msg);
  if (!status.ok()) {
    spdlog::error("Failed to write message to MCAP: {}", status.message);
    return false;
  }
  
  return true;
}

} // namespace larcs::runtime
