#include "larcs/sim/sensor/sensor_base.hpp"

namespace larcs::sim {

SensorBase::Transform SensorBase::get_world_transform(
    const Transform& robot_pose) const {
  Transform world_tf;

  // ロボット姿勢からワールド座標系でのセンサー位置を計算
  world_tf.position =
      robot_pose.position + robot_pose.orientation * mount_tf_.position;
  world_tf.orientation = robot_pose.orientation * mount_tf_.orientation;

  return world_tf;
}

}  // namespace larcs::sim
