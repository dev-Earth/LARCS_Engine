#include "larcs/sim/sensor/sensor_factory.hpp"

#include <yaml-cpp/yaml.h>

#include "larcs/sim/sensor/encoder_sensor.hpp"
#include "larcs/sim/sensor/imu_sensor.hpp"

namespace larcs::sim {

std::unique_ptr<SensorBase> SensorFactory::create_from_yaml(
    const std::string& type, const std::string& name,
    const YAML::Node& spec_node) {
  if (type == "encoder") {
    EncoderSensor::Spec spec;
    spec.model = spec_node["model"].as<std::string>("incremental");
    spec.resolution = spec_node["resolution"].as<int>(2048);
    spec.wheel_radius = spec_node["wheel_radius"].as<double>(0.05);

    std::string topic = spec_node["topic"].as<std::string>("/encoder/" + name);

    return std::make_unique<EncoderSensor>(name, spec, topic);
  } else if (type == "imu") {
    ImuSensor::Spec spec;
    spec.model = spec_node["model"].as<std::string>("generic");
    spec.accel_noise_stddev =
        spec_node["accel_noise_stddev"].as<double>(0.01);
    spec.gyro_noise_stddev = spec_node["gyro_noise_stddev"].as<double>(0.001);
    spec.update_rate = spec_node["update_rate"].as<double>(200.0);

    std::string topic = spec_node["topic"].as<std::string>("/imu/" + name);

    return std::make_unique<ImuSensor>(name, spec, topic);
  }

  return nullptr;
}

}  // namespace larcs::sim
