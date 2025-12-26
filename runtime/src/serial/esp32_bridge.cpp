#include "larcs/runtime/esp32_bridge.hpp"
#include "larcs/runtime/subscriber.hpp"
#include "larcs/runtime/publisher.hpp"
#include <spdlog/spdlog.h>

namespace larcs::runtime {

ESP32Bridge::ESP32Bridge(std::shared_ptr<ZenohTransport> transport,
                         const std::string& serial_device,
                         int baudrate)
    : transport_(transport),
      serial_(std::make_unique<SerialPort>(serial_device, baudrate)) {
}

ESP32Bridge::~ESP32Bridge() {
  stop();
}

bool ESP32Bridge::start() {
  if (running_.load()) {
    spdlog::warn("ESP32Bridge already running");
    return true;
  }
  
  if (!transport_ || !transport_->is_running()) {
    spdlog::error("Transport not initialized");
    return false;
  }
  
  if (!serial_->open()) {
    spdlog::error("Failed to open serial port");
    return false;
  }
  
  // Start async serial read
  serial_->start_async_read([this](const std::vector<uint8_t>& data) {
    this->on_serial_data(data);
  });
  
  // Subscribe to command topic
  auto command_sub = std::make_shared<Subscriber<larcs::msgs::ESP32Command>>(
      transport_, "/robot/esp32/command",
      [this](const larcs::msgs::ESP32Command& cmd) {
        this->on_command(cmd);
      });
  
  running_.store(true);
  spdlog::info("ESP32Bridge started");
  return true;
}

void ESP32Bridge::stop() {
  if (!running_.load()) {
    return;
  }
  
  running_.store(false);
  serial_->stop_async_read();
  serial_->close();
  
  spdlog::info("ESP32Bridge stopped");
}

void ESP32Bridge::on_serial_data(const std::vector<uint8_t>& data) {
  // Append data to frame buffer
  frame_buffer_.insert(frame_buffer_.end(), data.begin(), data.end());
  
  // Parse frames
  std::vector<uint8_t> payload;
  while (parse_frame(payload)) {
    // Deserialize ESP32State message
    larcs::msgs::ESP32State state;
    if (state.ParseFromArray(payload.data(), payload.size())) {
      // Publish to Zenoh
      auto publisher = std::make_shared<Publisher<larcs::msgs::ESP32State>>(
          transport_, "/robot/esp32/state");
      publisher->publish(state);
      
      spdlog::debug("Received ESP32State: battery={:.2f}V, estop={}",
                   state.battery_voltage(), state.estop_pressed());
    } else {
      spdlog::error("Failed to parse ESP32State message");
    }
    
    payload.clear();
  }
}

void ESP32Bridge::on_command(const larcs::msgs::ESP32Command& cmd) {
  // Serialize command
  std::vector<uint8_t> payload(cmd.ByteSizeLong());
  if (!cmd.SerializeToArray(payload.data(), payload.size())) {
    spdlog::error("Failed to serialize ESP32Command");
    return;
  }
  
  // Frame and send
  auto frame = frame_message(payload);
  if (!serial_->write(frame)) {
    spdlog::error("Failed to send ESP32Command");
    return;
  }
  
  spdlog::debug("Sent ESP32Command: mode={}, wheels={}", 
               static_cast<int>(cmd.mode()), cmd.wheel_rpm_target_size());
}

uint16_t ESP32Bridge::calculate_crc16(const std::vector<uint8_t>& data) {
  // CRC-16-CCITT (0xFFFF initial, 0x1021 polynomial)
  uint16_t crc = 0xFFFF;
  
  for (uint8_t byte : data) {
    crc ^= (static_cast<uint16_t>(byte) << 8);
    
    for (int i = 0; i < 8; ++i) {
      if (crc & 0x8000) {
        crc = (crc << 1) ^ 0x1021;
      } else {
        crc = crc << 1;
      }
    }
  }
  
  return crc;
}

std::vector<uint8_t> ESP32Bridge::frame_message(const std::vector<uint8_t>& payload) {
  std::vector<uint8_t> frame;
  
  // Start byte
  frame.push_back(FRAME_START);
  
  // Length (2 bytes, big-endian)
  uint16_t length = payload.size();
  frame.push_back((length >> 8) & 0xFF);
  frame.push_back(length & 0xFF);
  
  // Payload
  frame.insert(frame.end(), payload.begin(), payload.end());
  
  // CRC16
  uint16_t crc = calculate_crc16(payload);
  frame.push_back((crc >> 8) & 0xFF);
  frame.push_back(crc & 0xFF);
  
  return frame;
}

bool ESP32Bridge::parse_frame(std::vector<uint8_t>& payload) {
  // Find start byte
  auto start_it = std::find(frame_buffer_.begin(), frame_buffer_.end(), FRAME_START);
  
  if (start_it == frame_buffer_.end()) {
    // No start byte found, clear buffer
    frame_buffer_.clear();
    return false;
  }
  
  // Remove bytes before start
  if (start_it != frame_buffer_.begin()) {
    frame_buffer_.erase(frame_buffer_.begin(), start_it);
  }
  
  // Check if we have enough bytes for header (start + length)
  if (frame_buffer_.size() < 3) {
    return false;
  }
  
  // Parse length
  uint16_t length = (static_cast<uint16_t>(frame_buffer_[1]) << 8) | frame_buffer_[2];
  
  // Check if we have complete frame (start + length + payload + crc)
  size_t frame_size = 1 + 2 + length + 2;
  if (frame_buffer_.size() < frame_size) {
    return false;
  }
  
  // Extract payload
  payload.assign(frame_buffer_.begin() + 3, frame_buffer_.begin() + 3 + length);
  
  // Extract and verify CRC
  uint16_t received_crc = (static_cast<uint16_t>(frame_buffer_[3 + length]) << 8) |
                          frame_buffer_[3 + length + 1];
  uint16_t calculated_crc = calculate_crc16(payload);
  
  if (received_crc != calculated_crc) {
    spdlog::error("CRC mismatch: received={:04X}, calculated={:04X}", 
                 received_crc, calculated_crc);
    // Remove this frame and continue
    frame_buffer_.erase(frame_buffer_.begin(), frame_buffer_.begin() + frame_size);
    return false;
  }
  
  // Remove processed frame
  frame_buffer_.erase(frame_buffer_.begin(), frame_buffer_.begin() + frame_size);
  
  return true;
}

} // namespace larcs::runtime
