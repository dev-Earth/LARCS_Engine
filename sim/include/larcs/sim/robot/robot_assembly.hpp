#pragma once
#include <Eigen/Dense>
#include <memory>
#include <string>
#include <vector>

#include "larcs/sim/sensor/encoder_sensor.hpp"
#include "larcs/sim/sensor/sensor_base.hpp"
#include "larcs/sim/world.hpp"

// Forward declarations
namespace larcs::runtime {
class ZenohTransport;
}

namespace physx {
class PxRigidDynamic;
}

namespace larcs::sim {

class RobotAssembly {
 public:
  struct Wheel {
    std::string name;
    double radius;
    double width;
    physx::PxRigidDynamic* actor = nullptr;
    std::unique_ptr<EncoderSensor> encoder;
  };

  explicit RobotAssembly(const std::string& name);
  ~RobotAssembly();

  // ロボット定義読み込み
  bool load_from_yaml(const std::string& yaml_path);
  // USD support placeholder for Phase 2.2
  // bool load_from_usd(const std::string& usd_path);

  // Worldにスポーン
  bool spawn(World* world, const Eigen::Vector3d& position);

  // 更新
  void update(double sim_time);
  void publish_state(
      std::shared_ptr<larcs::runtime::ZenohTransport> transport);

  // 速度指令
  void set_velocity(const Eigen::Vector2d& linear, double angular);

  // センサー取得
  const std::vector<std::unique_ptr<SensorBase>>& get_sensors() const {
    return sensors_;
  }

 private:
  std::string name_;
  physx::PxRigidDynamic* base_actor_ = nullptr;
  std::vector<Wheel> wheels_;
  std::vector<std::unique_ptr<SensorBase>> sensors_;

  // 差動二輪制御
  void update_differential_drive(double dt);
  Eigen::Vector2d target_linear_{0.0, 0.0};
  double target_angular_ = 0.0;

  // Odometry計算
  Eigen::Vector3d odom_position_{0.0, 0.0, 0.0};
  Eigen::Quaterniond odom_orientation_{1.0, 0.0, 0.0, 0.0};
  Eigen::Vector3d odom_linear_vel_{0.0, 0.0, 0.0};
  Eigen::Vector3d odom_angular_vel_{0.0, 0.0, 0.0};
  void update_odometry(double dt);

  double last_update_time_ = 0.0;
};

}  // namespace larcs::sim
