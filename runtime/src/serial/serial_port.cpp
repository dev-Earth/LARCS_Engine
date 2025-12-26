#include "larcs/runtime/serial_port.hpp"
#include <spdlog/spdlog.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <sys/select.h>
#include <cstring>
#include <cerrno>

namespace larcs::runtime {

SerialPort::SerialPort(const std::string& device, int baudrate)
    : device_(device), baudrate_(baudrate) {
}

SerialPort::~SerialPort() {
  close();
}

bool SerialPort::open() {
  if (fd_ >= 0) {
    spdlog::warn("Serial port already open");
    return true;
  }
  
  // Open device
  fd_ = ::open(device_.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
  if (fd_ < 0) {
    spdlog::error("Failed to open serial device {}: {}", device_, std::strerror(errno));
    return false;
  }
  
  // Configure termios
  struct termios tty;
  if (tcgetattr(fd_, &tty) != 0) {
    spdlog::error("Failed to get terminal attributes: {}", std::strerror(errno));
    ::close(fd_);
    fd_ = -1;
    return false;
  }
  
  // Set baud rate
  speed_t speed;
  switch (baudrate_) {
    case 9600: speed = B9600; break;
    case 19200: speed = B19200; break;
    case 38400: speed = B38400; break;
    case 57600: speed = B57600; break;
    case 115200: speed = B115200; break;
    case 230400: speed = B230400; break;
    case 460800: speed = B460800; break;
    case 921600: speed = B921600; break;
    default:
      spdlog::error("Unsupported baud rate: {}", baudrate_);
      ::close(fd_);
      fd_ = -1;
      return false;
  }
  
  cfsetispeed(&tty, speed);
  cfsetospeed(&tty, speed);
  
  // 8N1 mode
  tty.c_cflag &= ~PARENB;  // No parity
  tty.c_cflag &= ~CSTOPB;  // 1 stop bit
  tty.c_cflag &= ~CSIZE;
  tty.c_cflag |= CS8;      // 8 data bits
  
  // Disable hardware flow control
  tty.c_cflag &= ~CRTSCTS;
  
  // Enable receiver, ignore modem control lines
  tty.c_cflag |= CREAD | CLOCAL;
  
  // Raw mode
  tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
  
  // Disable software flow control
  tty.c_iflag &= ~(IXON | IXOFF | IXANY);
  
  // Raw output
  tty.c_oflag &= ~OPOST;
  
  // Set read timeout (deciseconds)
  tty.c_cc[VMIN] = 0;
  tty.c_cc[VTIME] = 1;  // 100ms timeout
  
  // Apply settings
  if (tcsetattr(fd_, TCSANOW, &tty) != 0) {
    spdlog::error("Failed to set terminal attributes: {}", std::strerror(errno));
    ::close(fd_);
    fd_ = -1;
    return false;
  }
  
  // Flush buffers
  tcflush(fd_, TCIOFLUSH);
  
  spdlog::info("Opened serial port: {} at {} baud", device_, baudrate_);
  return true;
}

void SerialPort::close() {
  stop_async_read();
  
  if (fd_ >= 0) {
    ::close(fd_);
    fd_ = -1;
    spdlog::info("Closed serial port: {}", device_);
  }
}

bool SerialPort::is_open() const {
  return fd_ >= 0;
}

bool SerialPort::write(const std::vector<uint8_t>& data) {
  if (fd_ < 0) {
    spdlog::error("Serial port not open");
    return false;
  }
  
  ssize_t written = ::write(fd_, data.data(), data.size());
  if (written < 0) {
    spdlog::error("Failed to write to serial port: {}", std::strerror(errno));
    return false;
  }
  
  if (static_cast<size_t>(written) != data.size()) {
    spdlog::warn("Partial write to serial port: {}/{} bytes", written, data.size());
  }
  
  return true;
}

std::vector<uint8_t> SerialPort::read(size_t max_bytes, int timeout_ms) {
  std::vector<uint8_t> buffer;
  
  if (fd_ < 0) {
    spdlog::error("Serial port not open");
    return buffer;
  }
  
  fd_set read_fds;
  struct timeval timeout;
  
  FD_ZERO(&read_fds);
  FD_SET(fd_, &read_fds);
  
  timeout.tv_sec = timeout_ms / 1000;
  timeout.tv_usec = (timeout_ms % 1000) * 1000;
  
  int ret = select(fd_ + 1, &read_fds, nullptr, nullptr, &timeout);
  if (ret < 0) {
    spdlog::error("Select failed: {}", std::strerror(errno));
    return buffer;
  } else if (ret == 0) {
    // Timeout
    return buffer;
  }
  
  buffer.resize(max_bytes);
  ssize_t bytes_read = ::read(fd_, buffer.data(), max_bytes);
  
  if (bytes_read < 0) {
    spdlog::error("Failed to read from serial port: {}", std::strerror(errno));
    buffer.clear();
    return buffer;
  }
  
  buffer.resize(bytes_read);
  return buffer;
}

void SerialPort::start_async_read(ReadCallback callback) {
  if (running_.load()) {
    spdlog::warn("Async read already running");
    return;
  }
  
  if (fd_ < 0) {
    spdlog::error("Serial port not open");
    return;
  }
  
  running_.store(true);
  read_thread_ = std::thread(&SerialPort::read_loop, this, callback);
  spdlog::info("Started async read on serial port: {}", device_);
}

void SerialPort::stop_async_read() {
  if (!running_.load()) {
    return;
  }
  
  running_.store(false);
  
  if (read_thread_.joinable()) {
    read_thread_.join();
  }
  
  spdlog::info("Stopped async read on serial port: {}", device_);
}

void SerialPort::read_loop(ReadCallback callback) {
  const size_t BUFFER_SIZE = 1024;
  
  while (running_.load()) {
    auto data = read(BUFFER_SIZE, 100);
    
    if (!data.empty()) {
      callback(data);
    }
  }
}

} // namespace larcs::runtime
