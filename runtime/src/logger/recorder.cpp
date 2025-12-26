#include "larcs/runtime/recorder.hpp"
#include <spdlog/spdlog.h>
#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>
#include <fstream>

namespace larcs::runtime {

Recorder::Recorder(const std::string& filepath)
    : filepath_(filepath), writer_(std::make_unique<mcap::McapWriter>()) {
}

Recorder::~Recorder() {
  close();
}

bool Recorder::open() {
  std::lock_guard<std::mutex> lock(mutex_);
  
  if (open_) {
    spdlog::warn("Recorder already open");
    return true;
  }
  
  mcap::McapWriterOptions options("larcs");
  options.compression = mcap::Compression::None;  // Use no compression since LZ4 is disabled
  
  auto status = writer_->open(filepath_, options);
  if (!status.ok()) {
    spdlog::error("Failed to open MCAP file {}: {}", filepath_, status.message);
    return false;
  }
  
  open_ = true;
  spdlog::info("Opened MCAP recorder: {}", filepath_);
  return true;
}

void Recorder::close() {
  std::lock_guard<std::mutex> lock(mutex_);
  
  if (!open_) {
    return;
  }
  
  writer_->close();
  open_ = false;
  spdlog::info("Closed MCAP recorder: {}", filepath_);
}

bool Recorder::is_open() const {
  return open_;
}

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
