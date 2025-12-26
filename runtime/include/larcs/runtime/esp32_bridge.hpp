#pragma once
#include "larcs/runtime/serial_port.hpp"
#include "larcs/runtime/zenoh_transport.hpp"
#include "esp32.pb.h"
#include <memory>
#include <atomic>
#include <vector>

namespace larcs::runtime {

// PC ↔ ESP32 のプロトコル変換
class ESP32Bridge {
public:
  ESP32Bridge(std::shared_ptr<ZenohTransport> transport,
              const std::string& serial_device,
              int baudrate = 115200);
  ~ESP32Bridge();
  
  bool start();
  void stop();
  
private:
  void on_serial_data(const std::vector<uint8_t>& data);
  void on_command(const larcs::msgs::ESP32Command& cmd);
  
  uint16_t calculate_crc16(const std::vector<uint8_t>& data);
  std::vector<uint8_t> frame_message(const std::vector<uint8_t>& payload);
  bool parse_frame(std::vector<uint8_t>& payload);
  
  std::shared_ptr<ZenohTransport> transport_;
  std::unique_ptr<SerialPort> serial_;
  std::atomic<bool> running_{false};
  
  // フレーミング（区切り文字 0x7E + length + payload + CRC）
  std::vector<uint8_t> frame_buffer_;
  static constexpr uint8_t FRAME_START = 0x7E;
};

} // namespace larcs::runtime
