#pragma once
#include "larcs/runtime/zenoh_transport.hpp"
#include <mcap/reader.hpp>
#include <memory>
#include <string>
#include <vector>

namespace larcs::runtime {

class Replayer {
public:
  explicit Replayer(const std::string& filepath);
  ~Replayer();
  
  bool open();
  void close();
  
  struct ReplayOptions {
    double rate = 1.0;              // Playback rate multiplier
    bool loop = false;              // Loop playback
    uint64_t start_time_ns = 0;     // Start time
    uint64_t end_time_ns = UINT64_MAX;  // End time
  };
  
  // Execute replay (blocking)
  bool replay(std::shared_ptr<ZenohTransport> transport, const ReplayOptions& options);
  
  // Get file information
  struct FileInfo {
    uint64_t start_time_ns;
    uint64_t end_time_ns;
    uint64_t message_count;
    std::vector<std::string> topics;
  };
  FileInfo get_info() const;
  
private:
  std::string filepath_;
  std::unique_ptr<mcap::McapReader> reader_;
  bool open_ = false;
};

} // namespace larcs::runtime
