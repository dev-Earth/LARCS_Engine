#include "larcs/sim/sensor/encoder_sensor.hpp"

#include <cmath>

#include "larcs/runtime/zenoh_transport.hpp"

namespace larcs::sim {

EncoderSensor::EncoderSensor(const std::string& name, const Spec& spec,
                             const std::string& topic)
    : spec_(spec) {
  name_ = name;
  topic_ = topic;
}

void EncoderSensor::set_wheel_velocity(double rpm) { rpm_ = rpm; }

void EncoderSensor::update(double sim_time, const Transform& robot_pose) {
  if (last_update_time_ == 0.0) {
    last_update_time_ = sim_time;
    return;
  }

  double dt = sim_time - last_update_time_;
  last_update_time_ = sim_time;

  // RPMからティック数を計算
  // RPM = 回転数/分 → RPS = RPM / 60
  // ticks = RPS * resolution * dt
  double rps = rpm_ / 60.0;
  double ticks_delta = rps * spec_.resolution * dt;

  ticks_ += static_cast<int64_t>(ticks_delta);
}

void EncoderSensor::publish(
    std::shared_ptr<larcs::runtime::ZenohTransport> transport) {
  // Placeholder - will implement protobuf message in Phase 2.2
  // For now, just a stub that could send encoder data via Zenoh
  (void)transport;  // Suppress unused parameter warning
}

}  // namespace larcs::sim
