#include "larcs/sim/robot/robot_assembly.hpp"

#include <yaml-cpp/yaml.h>

#include <fstream>

#include "larcs/runtime/zenoh_transport.hpp"
#include "larcs/sim/sensor/sensor_factory.hpp"

namespace larcs::sim {

RobotAssembly::RobotAssembly(const std::string& name) : name_(name) {}

RobotAssembly::~RobotAssembly() = default;

bool RobotAssembly::load_from_yaml(const std::string& yaml_path) {
  try {
    YAML::Node config = YAML::LoadFile(yaml_path);

    if (!config["robot"]) {
      spdlog::error("No 'robot' section in config file");
      return false;
    }

    YAML::Node robot = config["robot"];

    // Load wheels
    if (robot["wheels"]) {
      for (const auto& wheel_node : robot["wheels"]) {
        Wheel wheel;
        wheel.name = wheel_node["name"].as<std::string>();
        wheel.radius = wheel_node["radius"].as<double>();
        wheel.width = wheel_node["width"].as<double>();

        // Create encoder if specified
        if (wheel_node["encoder"]) {
          wheel.encoder = std::unique_ptr<EncoderSensor>(
              static_cast<EncoderSensor*>(
                  SensorFactory::create_from_yaml("encoder", wheel.name,
                                                   wheel_node["encoder"])
                      .release()));
        }

        wheels_.push_back(std::move(wheel));
      }
    }

    // Load sensors
    if (robot["sensors"]) {
      for (const auto& sensor_node : robot["sensors"]) {
        std::string type = sensor_node["type"].as<std::string>();
        std::string name = sensor_node["name"].as<std::string>();

        auto sensor =
            SensorFactory::create_from_yaml(type, name, sensor_node["spec"]);
        if (sensor) {
          // Set mount transform
          if (sensor_node["position"]) {
            SensorBase::Transform tf;
            auto pos = sensor_node["position"];
            tf.position =
                Eigen::Vector3d(pos[0].as<double>(), pos[1].as<double>(),
                                pos[2].as<double>());

            if (sensor_node["orientation"]) {
              auto ori = sensor_node["orientation"];
              tf.orientation = Eigen::Quaterniond(
                  ori[3].as<double>(), ori[0].as<double>(),
                  ori[1].as<double>(), ori[2].as<double>());
            } else {
              tf.orientation = Eigen::Quaterniond::Identity();
            }

            sensor->set_mount_transform(tf);
          }

          sensors_.push_back(std::move(sensor));
        }
      }
    }

    spdlog::info("Loaded robot '{}' with {} wheels and {} sensors", name_,
                 wheels_.size(), sensors_.size());
    return true;

  } catch (const std::exception& e) {
    spdlog::error("Failed to load robot from YAML: {}", e.what());
    return false;
  }
}

bool RobotAssembly::spawn(World* world, const Eigen::Vector3d& position) {
  if (world == nullptr || world->get_scene() == nullptr) {
    spdlog::error("Cannot spawn robot: World not initialized");
    return false;
  }

  // TODO: Create PhysX actors for base and wheels
  // For now, just set initial odometry
  odom_position_ = position;
  odom_orientation_ = Eigen::Quaterniond::Identity();

  spdlog::info("Robot '{}' spawned at ({}, {}, {})", name_, position.x(),
               position.y(), position.z());
  return true;
}

void RobotAssembly::set_velocity(const Eigen::Vector2d& linear,
                                 double angular) {
  target_linear_ = linear;
  target_angular_ = angular;
}

void RobotAssembly::update_differential_drive(double dt) {
  // TODO: Implement differential drive kinematics
  // Convert linear and angular velocity to wheel velocities
  // Apply to PhysX actors

  (void)dt;  // Suppress warning
}

void RobotAssembly::update_odometry(double dt) {
  // TODO: Calculate odometry from wheel encoders
  // Update odom_position_, odom_orientation_, etc.

  (void)dt;  // Suppress warning
}

void RobotAssembly::update(double sim_time) {
  double dt = 0.0;
  if (last_update_time_ > 0.0) {
    dt = sim_time - last_update_time_;
  }
  last_update_time_ = sim_time;

  if (dt > 0.0) {
    update_differential_drive(dt);
    update_odometry(dt);
  }

  // Update all sensors
  SensorBase::Transform robot_pose;
  robot_pose.position = odom_position_;
  robot_pose.orientation = odom_orientation_;

  for (auto& sensor : sensors_) {
    sensor->update(sim_time, robot_pose);
  }

  for (auto& wheel : wheels_) {
    if (wheel.encoder) {
      wheel.encoder->update(sim_time, robot_pose);
    }
  }
}

void RobotAssembly::publish_state(
    std::shared_ptr<larcs::runtime::ZenohTransport> transport) {
  // Publish sensor data
  for (auto& sensor : sensors_) {
    sensor->publish(transport);
  }

  for (auto& wheel : wheels_) {
    if (wheel.encoder) {
      wheel.encoder->publish(transport);
    }
  }

  // TODO: Publish odometry
}

}  // namespace larcs::sim
