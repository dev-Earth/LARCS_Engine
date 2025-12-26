#pragma once
#include <Eigen/Dense>
#include <memory>
#include <string>

// Forward declarations to avoid requiring runtime headers in sim headers
namespace larcs::runtime {
class ZenohTransport;
}

namespace larcs::sim {

class SensorBase {
 public:
  struct Transform {
    Eigen::Vector3d position;
    Eigen::Quaterniond orientation;
  };

  virtual ~SensorBase() = default;

  // センサー更新（PhysXステップ後に呼ばれる）
  virtual void update(double sim_time, const Transform& robot_pose) = 0;

  // Zenoh経由でデータ配信
  virtual void publish(
      std::shared_ptr<larcs::runtime::ZenohTransport> transport) = 0;

  // センサー仕様
  virtual std::string get_type() const = 0;
  virtual std::string get_model() const = 0;
  virtual std::string get_topic() const = 0;

  // 取付位置（ロボット座標系からのオフセット）
  void set_mount_transform(const Transform& tf) { mount_tf_ = tf; }
  Transform get_mount_transform() const { return mount_tf_; }

  // ワールド座標系でのセンサー位置計算
  Transform get_world_transform(const Transform& robot_pose) const;

 protected:
  Transform mount_tf_;
  std::string name_;
  std::string topic_;
};

}  // namespace larcs::sim
