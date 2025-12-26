#pragma once
#include <memory>
#include <string>

namespace larcs::runtime {

enum class QoSProfile {
  Control,      // High reliability, low latency (trajectory, commands, E-stop)
  Telemetry,    // Latest value priority (state, statistics)
  Perception    // Best effort, large data (point clouds, images)
};

class Transport {
 public:
  virtual ~Transport() = default;
  virtual bool initialize(const std::string& config_path = "") = 0;
  virtual void shutdown() = 0;
  virtual bool is_running() const = 0;
};

}  // namespace larcs::runtime
