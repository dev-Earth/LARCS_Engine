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
  // Note: LZ4 compression is disabled by vcpkg build configuration (MCAP_COMPRESSION_NO_LZ4)
  // Using no compression to avoid runtime errors
  options.compression = mcap::Compression::None;
  
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

} // namespace larcs::runtime
