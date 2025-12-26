# Sensor Framework Documentation

## Overview

The LARCS sensor framework provides a modular, extensible system for simulating robot sensors. Each sensor implements a common interface while providing specialized functionality for different sensor types.

## Design Philosophy

### Sim/Real Consistency

Sensors in simulation produce the **same data format** as real sensors:
- Same message types (Protobuf)
- Same topics (Zenoh)
- Same update rates
- Same noise characteristics

This enables **seamless code reuse** between simulation and real robots.

### Modularity

Each sensor is:
- **Self-contained**: Independent update and publish
- **Configurable**: YAML-based specifications
- **Pluggable**: Easy to add new sensor types
- **Testable**: Unit tests for each sensor

## Core Classes

### SensorBase (Abstract)

Base class for all sensors:

```cpp
class SensorBase {
public:
  struct Transform {
    Eigen::Vector3d position;
    Eigen::Quaterniond orientation;
  };
  
  // Update sensor state
  virtual void update(double sim_time, const Transform& robot_pose) = 0;
  
  // Publish data via Zenoh
  virtual void publish(std::shared_ptr<ZenohTransport> transport) = 0;
  
  // Sensor information
  virtual std::string get_type() const = 0;
  virtual std::string get_model() const = 0;
  virtual std::string get_topic() const = 0;
  
  // Mount transform (robot frame → sensor frame)
  void set_mount_transform(const Transform& tf);
  Transform get_world_transform(const Transform& robot_pose) const;
};
```

**Key Methods:**

- `update()`: Called every simulation step, updates internal state
- `publish()`: Sends data via Zenoh (called at sensor's update rate)
- `get_world_transform()`: Calculates sensor pose in world frame

### Transform Calculation

Sensors are mounted relative to robot base:

```
World Frame
    ↓ robot_pose
Robot Base Frame
    ↓ mount_transform
Sensor Frame
```

Implemented in `get_world_transform()`:
```cpp
world_position = robot_position + robot_orientation * mount_position
world_orientation = robot_orientation * mount_orientation
```

## Sensor Types

### Phase 2.1 (Current)

#### EncoderSensor

Wheel encoder for odometry:

**Specification:**
```yaml
encoder:
  model: "incremental"
  resolution: 2048  # PPR (Pulses Per Revolution)
  wheel_radius: 0.05  # meters
  topic: "/robot/encoder/left"
```

**Implementation:**
- Tracks tick count based on wheel RPM
- Accounts for resolution (PPR)
- Handles overflow (int64_t wrapping)

**Usage:**
```cpp
EncoderSensor::Spec spec{"incremental", 2048, 0.05};
auto encoder = std::make_unique<EncoderSensor>("left_wheel", spec, "/encoder/left");

// In simulation loop:
encoder->set_wheel_velocity(rpm);
encoder->update(sim_time, robot_pose);
encoder->publish(transport);
```

**Output:** Tick count (int64_t)

#### ImuSensor

Inertial Measurement Unit:

**Specification:**
```yaml
imu:
  model: "BMI088"
  accel_noise_stddev: 0.01  # m/s²
  gyro_noise_stddev: 0.001  # rad/s
  update_rate: 200  # Hz
  topic: "/robot/imu"
```

**Implementation:**
- Simulates accelerometer (linear acceleration)
- Simulates gyroscope (angular velocity)
- Adds Gaussian noise
- Rate-limited updates (respects update_rate)

**Noise Model:**
```cpp
noisy_value = true_value + N(0, noise_stddev)
```

**Usage:**
```cpp
ImuSensor::Spec spec{"BMI088", 0.01, 0.001, 200.0};
auto imu = std::make_unique<ImuSensor>("imu_base", spec, "/imu");

// In simulation:
imu->set_acceleration(accel_vector);
imu->set_angular_velocity(gyro_vector);
imu->update(sim_time, robot_pose);
imu->publish(transport);
```

**Output:** 
- Linear acceleration (Vector3)
- Angular velocity (Vector3)

### Phase 2.2 (Planned)

#### LidarSensor

2D/3D LiDAR:

**Specification:**
```yaml
lidar:
  model: "VLP-16"
  channels: 16
  range_max: 100.0  # meters
  range_min: 0.3
  fov: 360  # degrees
  update_rate: 10  # Hz
  noise_stddev: 0.02
```

**Implementation:**
- PhysX raycast for distance measurement
- Multiple scan lines (channels)
- Range noise model
- Returns PointCloud2 message

#### DepthCameraSensor

RGB-D camera (like RealSense):

**Specification:**
```yaml
depth_camera:
  model: "D435"
  width: 640
  height: 480
  fov_h: 69  # degrees
  fov_v: 42
  range_max: 10.0
  update_rate: 30  # Hz
```

**Implementation:**
- GPU raycast for depth image
- Parallel projection
- Depth noise (increase with distance)
- Returns Image message

### Phase 2.3+ (Future)

- **CameraSensor**: RGB camera with lens distortion
- **GpsSensor**: GPS/GNSS with position noise
- **UltrasonicSensor**: Short-range distance sensor
- **MagnetometerSensor**: Magnetic field (compass)
- **BarometerSensor**: Altitude from pressure
- **ForceSensor**: Contact force measurement
- **TorqueSensor**: Joint torque measurement

## Sensor Factory

Creates sensors from configuration:

```cpp
class SensorFactory {
public:
  static std::unique_ptr<SensorBase> create_from_yaml(
    const std::string& type,
    const std::string& name,
    const YAML::Node& spec_node
  );
};
```

**Usage:**
```cpp
YAML::Node config = YAML::LoadFile("robot.yaml");
for (const auto& sensor_node : config["sensors"]) {
  std::string type = sensor_node["type"].as<std::string>();
  std::string name = sensor_node["name"].as<std::string>();
  
  auto sensor = SensorFactory::create_from_yaml(type, name, sensor_node["spec"]);
  sensors.push_back(std::move(sensor));
}
```

## Configuration Format

### Complete Sensor Config

```yaml
sensors:
  - name: "imu_base"
    type: "imu"
    position: [0, 0, 0.05]  # relative to robot base
    orientation: [0, 0, 0, 1]  # quaternion [x, y, z, w]
    spec:
      model: "BMI088"
      accel_noise_stddev: 0.01
      gyro_noise_stddev: 0.001
      update_rate: 200
      topic: "/robot/imu"
  
  - name: "lidar_front"
    type: "lidar"
    position: [0.15, 0, 0.1]
    orientation: [0, 0, 0, 1]
    spec:
      model: "VLP-16"
      channels: 16
      range_max: 100.0
      update_rate: 10
      topic: "/robot/scan"
```

## Adding Custom Sensors

### Step 1: Define Sensor Class

```cpp
// include/larcs/sim/sensor/my_sensor.hpp
#pragma once
#include "larcs/sim/sensor/sensor_base.hpp"

namespace larcs::sim {

class MySensor : public SensorBase {
public:
  struct Spec {
    std::string model;
    double custom_param;
    // ... sensor-specific parameters
  };
  
  MySensor(const std::string& name, const Spec& spec, const std::string& topic);
  
  void update(double sim_time, const Transform& robot_pose) override;
  void publish(std::shared_ptr<ZenohTransport> transport) override;
  
  std::string get_type() const override { return "my_sensor"; }
  std::string get_model() const override { return spec_.model; }
  std::string get_topic() const override { return topic_; }

private:
  Spec spec_;
  // ... internal state
};

}  // namespace larcs::sim
```

### Step 2: Implement Sensor

```cpp
// src/sensor/my_sensor.cpp
#include "larcs/sim/sensor/my_sensor.hpp"

namespace larcs::sim {

MySensor::MySensor(const std::string& name, const Spec& spec, 
                   const std::string& topic)
    : spec_(spec) {
  name_ = name;
  topic_ = topic;
}

void MySensor::update(double sim_time, const Transform& robot_pose) {
  // Update sensor state based on:
  // - sim_time: current simulation time
  // - robot_pose: robot pose in world frame
  // - mount_tf_: sensor mount offset (use get_world_transform())
}

void MySensor::publish(std::shared_ptr<ZenohTransport> transport) {
  // Create protobuf message
  // Publish via transport->publish()
}

}  // namespace larcs::sim
```

### Step 3: Register in Factory

```cpp
// src/sensor/sensor_factory.cpp
#include "larcs/sim/sensor/my_sensor.hpp"

std::unique_ptr<SensorBase> SensorFactory::create_from_yaml(
    const std::string& type, const std::string& name,
    const YAML::Node& spec_node) {
  
  // ... existing sensors ...
  
  if (type == "my_sensor") {
    MySensor::Spec spec;
    spec.model = spec_node["model"].as<std::string>();
    spec.custom_param = spec_node["custom_param"].as<double>();
    
    std::string topic = spec_node["topic"].as<std::string>("/my_sensor/" + name);
    return std::make_unique<MySensor>(name, spec, topic);
  }
  
  return nullptr;
}
```

### Step 4: Add to Build System

```cmake
# sim/CMakeLists.txt
set(SIM_SOURCES
  # ...
  src/sensor/my_sensor.cpp
)

set(SIM_HEADERS
  # ...
  include/larcs/sim/sensor/my_sensor.hpp
)
```

### Step 5: Write Tests

```cpp
// sim/tests/my_sensor_test.cpp
#include "larcs/sim/sensor/my_sensor.hpp"
#include <gtest/gtest.h>

TEST(MySensorTest, BasicFunctionality) {
  MySensor::Spec spec{"test_model", 1.0};
  MySensor sensor("test", spec, "/test");
  
  // Test sensor behavior
  EXPECT_EQ(sensor.get_type(), "my_sensor");
  // ...
}
```

## Best Practices

### Noise Modeling

Use realistic noise parameters based on real sensor datasheets:

```cpp
// IMU: BMI088 datasheet
accel_noise_density = 180 µg/√Hz  // → stddev ≈ 0.01 m/s²
gyro_noise_density = 0.014 °/s/√Hz  // → stddev ≈ 0.001 rad/s
```

### Update Rates

Match real sensor rates:
- IMU: 100-500 Hz
- Encoder: 100-240 Hz (or per simulation step)
- LiDAR: 5-20 Hz
- Camera: 15-60 Hz
- GPS: 1-10 Hz

### Coordinate Frames

Follow ROS conventions:
- X: forward
- Y: left
- Z: up
- Rotations: right-hand rule

### Testing

Test each sensor independently:
```cpp
TEST(SensorTest, UpdateRate) {
  // Verify sensor respects configured update rate
}

TEST(SensorTest, NoiseCharacteristics) {
  // Verify noise has correct statistics
}

TEST(SensorTest, TransformCalculation) {
  // Verify world transform is calculated correctly
}
```

## Performance Considerations

### Update vs Publish

Separate update (every step) from publish (at sensor rate):
```cpp
void update(double sim_time, const Transform& robot_pose) override {
  // Fast: update internal state every step
  calculate_sensor_value();
}

void publish(std::shared_ptr<ZenohTransport> transport) override {
  // Slow: only publish at sensor update rate
  if (time_since_last_publish >= 1.0 / update_rate_) {
    transport->publish(topic_, message);
  }
}
```

### Expensive Sensors

For computationally expensive sensors (LiDAR, cameras):
- Use lower update rates
- Implement LOD (level of detail)
- Consider async processing
- Profile and optimize

## Debugging

### Visualize Sensor Frames

Use RViz or custom tool to visualize:
- Sensor mount positions
- Sensor orientations
- Sensor FOV/range

### Log Sensor Data

```cpp
void publish(...) override {
  spdlog::debug("Sensor {}: value={}", name_, current_value);
  // ... publish
}
```

### Unit Tests

Each sensor should have:
- Construction test
- Update test
- Noise test
- Rate test
- Transform test

## Future Extensions

### Sensor Fusion

Combine multiple sensors:
```cpp
class SensorFusion {
  void add_sensor(std::unique_ptr<SensorBase> sensor);
  void update(double sim_time);
  EstimatedState get_fused_state();
};
```

### Sensor Failures

Simulate realistic failures:
```cpp
class SensorBase {
  void set_failure_mode(FailureMode mode);
  bool is_healthy() const;
};
```

### Sensor Calibration

Support calibration parameters:
```yaml
sensor:
  calibration:
    bias: [0.01, -0.02, 0.00]
    scale: [1.01, 0.99, 1.00]
    misalignment: [0.001, 0.002, 0.000]
```

## Resources

- Sensor datasheets for noise parameters
- ROS sensor_msgs for message formats
- Real robot sensor integration code
- Academic papers on sensor modeling

## Examples

See `sim/tests/sensor_test.cpp` for complete examples of sensor usage and testing.
