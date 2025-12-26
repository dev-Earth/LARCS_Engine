#pragma once
#include "larcs/sim/sensor/sensor_base.hpp"

namespace larcs::sim {

class EncoderSensor : public SensorBase {
 public:
  struct Spec {
    std::string model;
    int resolution;      // PPR (Pulses Per Revolution)
    double wheel_radius;  // [m]
  };

  EncoderSensor(const std::string& name, const Spec& spec,
                const std::string& topic);

  void update(double sim_time, const Transform& robot_pose) override;
  void publish(
      std::shared_ptr<larcs::runtime::ZenohTransport> transport) override;

  std::string get_type() const override { return "encoder"; }
  std::string get_model() const override { return spec_.model; }
  std::string get_topic() const override { return topic_; }

  // 車輪速度設定（シミュレーション内部から呼ばれる）
  void set_wheel_velocity(double rpm);

  int64_t get_ticks() const { return ticks_; }

 private:
  Spec spec_;
  int64_t ticks_ = 0;
  double rpm_ = 0.0;
  double last_update_time_ = 0.0;
};

}  // namespace larcs::sim
