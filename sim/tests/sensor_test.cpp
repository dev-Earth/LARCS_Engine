#include <gtest/gtest.h>

#include "larcs/sim/sensor/encoder_sensor.hpp"
#include "larcs/sim/sensor/imu_sensor.hpp"

TEST(EncoderSensorTest, Construction) {
  larcs::sim::EncoderSensor::Spec spec{"test_encoder", 2048, 0.05};
  larcs::sim::EncoderSensor encoder("left_wheel", spec, "/test/encoder");

  EXPECT_EQ(encoder.get_type(), "encoder");
  EXPECT_EQ(encoder.get_model(), "test_encoder");
  EXPECT_EQ(encoder.get_topic(), "/test/encoder");
  EXPECT_EQ(encoder.get_ticks(), 0);
}

TEST(EncoderSensorTest, TickAccumulation) {
  larcs::sim::EncoderSensor::Spec spec{"test", 2048, 0.05};
  larcs::sim::EncoderSensor encoder("left_wheel", spec, "/test/encoder");

  larcs::sim::SensorBase::Transform tf;
  tf.position = Eigen::Vector3d(0, 0, 0);
  tf.orientation = Eigen::Quaterniond::Identity();

  encoder.set_wheel_velocity(60.0);  // 60 RPM

  // First update initializes time
  encoder.update(0.0, tf);
  EXPECT_EQ(encoder.get_ticks(), 0);

  // Second update calculates ticks
  encoder.update(0.1, tf);

  // 60 RPM = 1 RPS, 0.1s → 0.1回転 → 2048 * 0.1 = 204.8 ticks
  EXPECT_GT(encoder.get_ticks(), 200);
  EXPECT_LT(encoder.get_ticks(), 210);
}

TEST(EncoderSensorTest, ContinuousRotation) {
  larcs::sim::EncoderSensor::Spec spec{"test", 1000, 0.05};
  larcs::sim::EncoderSensor encoder("wheel", spec, "/test/encoder");

  larcs::sim::SensorBase::Transform tf;
  tf.position = Eigen::Vector3d(0, 0, 0);
  tf.orientation = Eigen::Quaterniond::Identity();

  encoder.set_wheel_velocity(120.0);  // 120 RPM

  encoder.update(0.0, tf);

  for (int i = 1; i <= 10; ++i) {
    encoder.update(i * 0.01, tf);
  }

  // 120 RPM = 2 RPS, 0.1s → 0.2回転 → 1000 * 0.2 = 200 ticks
  EXPECT_GT(encoder.get_ticks(), 195);
  EXPECT_LT(encoder.get_ticks(), 205);
}

TEST(ImuSensorTest, Construction) {
  larcs::sim::ImuSensor::Spec spec{"BMI088", 0.01, 0.001, 200};
  larcs::sim::ImuSensor imu("imu_base", spec, "/test/imu");

  EXPECT_EQ(imu.get_type(), "imu");
  EXPECT_EQ(imu.get_model(), "BMI088");
  EXPECT_EQ(imu.get_topic(), "/test/imu");
}

TEST(ImuSensorTest, SetAndUpdate) {
  larcs::sim::ImuSensor::Spec spec{"BMI088", 0.01, 0.001, 200};
  larcs::sim::ImuSensor imu("imu_base", spec, "/test/imu");

  Eigen::Vector3d accel(0, 0, 9.81);
  Eigen::Vector3d gyro(0, 0, 0);

  imu.set_acceleration(accel);
  imu.set_angular_velocity(gyro);

  larcs::sim::SensorBase::Transform tf;
  tf.position = Eigen::Vector3d(0, 0, 0);
  tf.orientation = Eigen::Quaterniond::Identity();

  imu.update(0.0, tf);
  imu.update(0.005, tf);  // 5ms = 200Hz

  SUCCEED();  // Just ensure no crashes
}

TEST(SensorBaseTest, WorldTransform) {
  larcs::sim::EncoderSensor::Spec spec{"test", 2048, 0.05};
  larcs::sim::EncoderSensor encoder("test", spec, "/test");

  // Set mount transform
  larcs::sim::SensorBase::Transform mount;
  mount.position = Eigen::Vector3d(0.1, 0.0, 0.05);
  mount.orientation = Eigen::Quaterniond::Identity();
  encoder.set_mount_transform(mount);

  // Robot pose
  larcs::sim::SensorBase::Transform robot_pose;
  robot_pose.position = Eigen::Vector3d(1.0, 2.0, 0.0);
  robot_pose.orientation = Eigen::Quaterniond::Identity();

  // Calculate world transform
  auto world_tf = encoder.get_world_transform(robot_pose);

  EXPECT_NEAR(world_tf.position.x(), 1.1, 0.001);
  EXPECT_NEAR(world_tf.position.y(), 2.0, 0.001);
  EXPECT_NEAR(world_tf.position.z(), 0.05, 0.001);
}
