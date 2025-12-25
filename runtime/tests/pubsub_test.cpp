#include "larcs/runtime/publisher.hpp"
#include "larcs/runtime/subscriber.hpp"
#include "larcs/runtime/zenoh_transport.hpp"
#include "larcs/msgs/geometry.pb.h"

#include <gtest/gtest.h>

#include <chrono>
#include <thread>

TEST(PubSubTest, TwistMessage) {
  auto transport = std::make_shared<larcs::runtime::ZenohTransport>();
  ASSERT_TRUE(transport->initialize());

  bool received = false;
  larcs::msgs::Twist received_msg;

  auto sub = std::make_shared<larcs::runtime::Subscriber<larcs::msgs::Twist>>(
      transport, "/test/twist", [&](const larcs::msgs::Twist& msg) {
        received = true;
        received_msg = msg;
      });

  auto pub = std::make_shared<larcs::runtime::Publisher<larcs::msgs::Twist>>(
      transport, "/test/twist");

  // Give time for subscription to be established
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  larcs::msgs::Twist twist;
  twist.mutable_linear()->set_x(1.5);
  twist.mutable_angular()->set_z(0.5);

  ASSERT_TRUE(pub->publish(twist));

  // Wait for message to be received
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  EXPECT_TRUE(received);
  if (received) {
    EXPECT_DOUBLE_EQ(received_msg.linear().x(), 1.5);
    EXPECT_DOUBLE_EQ(received_msg.angular().z(), 0.5);
  }

  transport->shutdown();
}

TEST(PubSubTest, MultipleMessages) {
  auto transport = std::make_shared<larcs::runtime::ZenohTransport>();
  ASSERT_TRUE(transport->initialize());

  int count = 0;
  double last_x = 0.0;

  auto sub = std::make_shared<larcs::runtime::Subscriber<larcs::msgs::Twist>>(
      transport, "/test/multi", [&](const larcs::msgs::Twist& msg) {
        count++;
        last_x = msg.linear().x();
      });

  auto pub = std::make_shared<larcs::runtime::Publisher<larcs::msgs::Twist>>(
      transport, "/test/multi");

  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  // Publish multiple messages
  for (int i = 0; i < 5; i++) {
    larcs::msgs::Twist twist;
    twist.mutable_linear()->set_x(i * 1.0);
    ASSERT_TRUE(pub->publish(twist));
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
  }

  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  EXPECT_EQ(count, 5);
  EXPECT_DOUBLE_EQ(last_x, 4.0);

  transport->shutdown();
}

TEST(PubSubTest, QoSProfiles) {
  auto transport = std::make_shared<larcs::runtime::ZenohTransport>();
  ASSERT_TRUE(transport->initialize());

  bool control_received = false;
  bool telemetry_received = false;

  auto control_sub =
      std::make_shared<larcs::runtime::Subscriber<larcs::msgs::Twist>>(
          transport, "/test/control",
          [&](const larcs::msgs::Twist& msg) { control_received = true; },
          larcs::runtime::QoSProfile::Control);

  auto telemetry_sub =
      std::make_shared<larcs::runtime::Subscriber<larcs::msgs::Twist>>(
          transport, "/test/telemetry",
          [&](const larcs::msgs::Twist& msg) { telemetry_received = true; },
          larcs::runtime::QoSProfile::Telemetry);

  auto control_pub =
      std::make_shared<larcs::runtime::Publisher<larcs::msgs::Twist>>(
          transport, "/test/control", larcs::runtime::QoSProfile::Control);

  auto telemetry_pub =
      std::make_shared<larcs::runtime::Publisher<larcs::msgs::Twist>>(
          transport, "/test/telemetry", larcs::runtime::QoSProfile::Telemetry);

  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  larcs::msgs::Twist twist;
  twist.mutable_linear()->set_x(1.0);

  ASSERT_TRUE(control_pub->publish(twist));
  ASSERT_TRUE(telemetry_pub->publish(twist));

  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  EXPECT_TRUE(control_received);
  EXPECT_TRUE(telemetry_received);

  transport->shutdown();
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
