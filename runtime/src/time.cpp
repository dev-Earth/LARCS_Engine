#include "larcs/runtime/time.hpp"

#include <ctime>

namespace larcs::runtime {

Time GetMonotonicTime() {
  auto now = std::chrono::steady_clock::now();
  auto duration = now.time_since_epoch();
  auto sec = std::chrono::duration_cast<std::chrono::seconds>(duration);
  auto nanosec = std::chrono::duration_cast<std::chrono::nanoseconds>(duration - sec);

  return Time(sec.count(), static_cast<uint32_t>(nanosec.count()));
}

Time GetSystemTime() {
  auto now = std::chrono::system_clock::now();
  auto duration = now.time_since_epoch();
  auto sec = std::chrono::duration_cast<std::chrono::seconds>(duration);
  auto nanosec = std::chrono::duration_cast<std::chrono::nanoseconds>(duration - sec);

  return Time(sec.count(), static_cast<uint32_t>(nanosec.count()));
}

double ToSeconds(const Time& time) {
  return static_cast<double>(time.sec) + static_cast<double>(time.nanosec) / 1e9;
}

Time FromSeconds(double seconds) {
  int64_t sec = static_cast<int64_t>(seconds);
  uint32_t nanosec = static_cast<uint32_t>((seconds - static_cast<double>(sec)) * 1e9);
  return Time(sec, nanosec);
}

Time FromTimePoint(const std::chrono::time_point<std::chrono::system_clock>& tp) {
  auto duration = tp.time_since_epoch();
  auto sec = std::chrono::duration_cast<std::chrono::seconds>(duration);
  auto nanosec = std::chrono::duration_cast<std::chrono::nanoseconds>(duration - sec);

  return Time(sec.count(), static_cast<uint32_t>(nanosec.count()));
}

std::chrono::time_point<std::chrono::system_clock> ToTimePoint(const Time& time) {
  auto sec = std::chrono::seconds(time.sec);
  auto nanosec = std::chrono::nanoseconds(time.nanosec);
  return std::chrono::time_point<std::chrono::system_clock>(sec + nanosec);
}

}  // namespace larcs::runtime
