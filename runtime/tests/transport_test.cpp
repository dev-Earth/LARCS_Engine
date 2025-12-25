#include "larcs/runtime/zenoh_transport.hpp"

#include <gtest/gtest.h>

TEST(ZenohTransportTest, InitializeAndShutdown) {
  auto transport = std::make_shared<larcs::runtime::ZenohTransport>();
  ASSERT_TRUE(transport->initialize());
  EXPECT_TRUE(transport->is_running());
  transport->shutdown();
  EXPECT_FALSE(transport->is_running());
}

TEST(ZenohTransportTest, DoubleInitialize) {
  auto transport = std::make_shared<larcs::runtime::ZenohTransport>();
  ASSERT_TRUE(transport->initialize());
  EXPECT_TRUE(transport->is_running());

  // Second initialize should succeed (no-op)
  EXPECT_TRUE(transport->initialize());
  EXPECT_TRUE(transport->is_running());

  transport->shutdown();
  EXPECT_FALSE(transport->is_running());
}

TEST(ZenohTransportTest, DoubleShutdown) {
  auto transport = std::make_shared<larcs::runtime::ZenohTransport>();
  ASSERT_TRUE(transport->initialize());
  transport->shutdown();
  EXPECT_FALSE(transport->is_running());

  // Second shutdown should be safe (no-op)
  transport->shutdown();
  EXPECT_FALSE(transport->is_running());
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
