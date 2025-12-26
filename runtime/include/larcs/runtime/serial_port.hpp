#pragma once
#include <string>
#include <vector>
#include <functional>
#include <thread>
#include <atomic>

namespace larcs::runtime {

class SerialPort {
public:
  explicit SerialPort(const std::string& device, int baudrate = 115200);
  ~SerialPort();
  
  bool open();
  void close();
  bool is_open() const;
  
  // 同期送受信
  bool write(const std::vector<uint8_t>& data);
  std::vector<uint8_t> read(size_t max_bytes, int timeout_ms = 100);
  
  // 非同期受信（コールバック）
  using ReadCallback = std::function<void(const std::vector<uint8_t>&)>;
  void start_async_read(ReadCallback callback);
  void stop_async_read();
  
private:
  std::string device_;
  int baudrate_;
  int fd_ = -1;
  std::atomic<bool> running_{false};
  std::thread read_thread_;
  
  void read_loop(ReadCallback callback);
};

} // namespace larcs::runtime
