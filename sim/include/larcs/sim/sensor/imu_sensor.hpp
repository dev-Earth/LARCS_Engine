#pragma once
#include <Eigen/Dense>

#include "larcs/sim/sensor/sensor_base.hpp"

namespace larcs::sim {

class ImuSensor : public SensorBase {
 public:
  struct Spec {
    std::string model;
    double accel_noise_stddev;  // [m/s^2]
    double gyro_noise_stddev;   // [rad/s]
    double update_rate;         // [Hz]
  };

  ImuSensor(const std::string& name, const Spec& spec,
            const std::string& topic);

  void update(double sim_time, const Transform& robot_pose) override;
  void publish(
      std::shared_ptr<larcs::runtime::ZenohTransport> transport) override;

  std::string get_type() const override { return "imu"; }
  std::string get_model() const override { return spec_.model; }
  std::string get_topic() const override { return topic_; }

  // PhysXから加速度・角速度を設定
  void set_acceleration(const Eigen::Vector3d& accel);
  void set_angular_velocity(const Eigen::Vector3d& gyro);

 private:
  Spec spec_;
  Eigen::Vector3d accel_;
  Eigen::Vector3d gyro_;
  double last_update_time_ = 0.0;

  // ノイズ生成
  Eigen::Vector3d add_noise(const Eigen::Vector3d& value, double stddev);
};

}  // namespace larcs::sim
