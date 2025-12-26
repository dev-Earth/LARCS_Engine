#include "larcs/sim/sensor/imu_sensor.hpp"

#include <random>

#include "larcs/runtime/zenoh_transport.hpp"

namespace larcs::sim {

ImuSensor::ImuSensor(const std::string& name, const Spec& spec,
                     const std::string& topic)
    : spec_(spec), accel_(0, 0, 0), gyro_(0, 0, 0) {
  name_ = name;
  topic_ = topic;
}

void ImuSensor::set_acceleration(const Eigen::Vector3d& accel) {
  accel_ = accel;
}

void ImuSensor::set_angular_velocity(const Eigen::Vector3d& gyro) {
  gyro_ = gyro;
}

void ImuSensor::update(double sim_time, const Transform& robot_pose) {
  if (last_update_time_ == 0.0) {
    last_update_time_ = sim_time;
    return;
  }

  double dt = sim_time - last_update_time_;

  // 更新レートに応じて更新
  if (dt < 1.0 / spec_.update_rate) {
    return;
  }

  last_update_time_ = sim_time;

  // ノイズを加えた値を内部に保存（publishで使用）
  // 実際の実装ではここでノイズを加えた値を保持
  (void)robot_pose;  // Suppress unused warning
}

Eigen::Vector3d ImuSensor::add_noise(const Eigen::Vector3d& value,
                                     double stddev) {
  static std::random_device rd;
  static std::mt19937 gen(rd());
  std::normal_distribution<double> dist(0.0, stddev);

  return Eigen::Vector3d(value.x() + dist(gen), value.y() + dist(gen),
                         value.z() + dist(gen));
}

void ImuSensor::publish(
    std::shared_ptr<larcs::runtime::ZenohTransport> transport) {
  // Placeholder - will implement protobuf message in Phase 2.2
  // Add noise and publish IMU data
  Eigen::Vector3d noisy_accel = add_noise(accel_, spec_.accel_noise_stddev);
  Eigen::Vector3d noisy_gyro = add_noise(gyro_, spec_.gyro_noise_stddev);

  (void)transport;  // Suppress unused parameter warning
  (void)noisy_accel;
  (void)noisy_gyro;
}

}  // namespace larcs::sim
