#pragma once

#include <chrono>
#include <cstdint>

namespace larcs::runtime {

// Time structure matching protobuf Time message
struct Time {
  int64_t sec;       // Seconds since epoch
  uint32_t nanosec;  // Nanoseconds component

  Time() : sec(0), nanosec(0) {
  }
  Time(int64_t s, uint32_t ns) : sec(s), nanosec(ns) {
  }
};

// Get current monotonic time
Time GetMonotonicTime();

// Get current system (wall clock) time
Time GetSystemTime();

// Convert Time to seconds (double)
double ToSeconds(const Time& time);

// Convert seconds (double) to Time
Time FromSeconds(double seconds);

// Convert std::chrono::time_point to Time
Time FromTimePoint(const std::chrono::time_point<std::chrono::system_clock>& tp);

// Convert Time to std::chrono::time_point
std::chrono::time_point<std::chrono::system_clock> ToTimePoint(const Time& time);

}  // namespace larcs::runtime
