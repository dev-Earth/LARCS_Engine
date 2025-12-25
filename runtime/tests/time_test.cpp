#include "larcs/runtime/time.hpp"

#include <gtest/gtest.h>

#include <thread>

using namespace larcs::runtime;

TEST(TimeTest, GetMonotonicTimeReturnsNonZero) {
  Time t = GetMonotonicTime();
  EXPECT_GT(t.sec, 0);
}

TEST(TimeTest, GetSystemTimeReturnsNonZero) {
  Time t = GetSystemTime();
  EXPECT_GT(t.sec, 0);
}

TEST(TimeTest, ToSecondsConversion) {
  Time t(10, 500000000);  // 10.5 seconds
  double seconds = ToSeconds(t);
  EXPECT_DOUBLE_EQ(seconds, 10.5);
}

TEST(TimeTest, FromSecondsConversion) {
  Time t = FromSeconds(10.5);
  EXPECT_EQ(t.sec, 10);
  EXPECT_EQ(t.nanosec, 500000000);
}

TEST(TimeTest, RoundTripConversion) {
  Time original(42, 123456789);
  double seconds = ToSeconds(original);
  Time converted = FromSeconds(seconds);

  EXPECT_EQ(converted.sec, original.sec);
  // Allow small rounding error in nanoseconds
  EXPECT_NEAR(converted.nanosec, original.nanosec, 1);
}

TEST(TimeTest, TimePointConversion) {
  auto now = std::chrono::system_clock::now();
  Time t = FromTimePoint(now);
  auto converted = ToTimePoint(t);

  // Times should be very close (within microseconds)
  auto diff = std::chrono::duration_cast<std::chrono::microseconds>(
      now - converted);
  EXPECT_LT(std::abs(diff.count()), 1000);
}

TEST(TimeTest, MonotonicTimeIncreases) {
  Time t1 = GetMonotonicTime();
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  Time t2 = GetMonotonicTime();

  double s1 = ToSeconds(t1);
  double s2 = ToSeconds(t2);

  EXPECT_GT(s2, s1);
  EXPECT_GE(s2 - s1, 0.01);  // At least 10ms difference
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
