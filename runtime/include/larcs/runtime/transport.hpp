#pragma once
#include <memory>
#include <string>

namespace larcs::runtime {

enum class QoSProfile {
  Control,      // 高信頼、低遅延（軌道、指令、E-stop）
  Telemetry,    // 最新優先（状態、統計）
  Perception    // ベストエフォート、大容量（点群、画像）
};

class Transport {
 public:
  virtual ~Transport() = default;
  virtual bool initialize(const std::string& config_path = "") = 0;
  virtual void shutdown() = 0;
  virtual bool is_running() const = 0;
};

}  // namespace larcs::runtime
