# LARCS Messaging System

## Overview

LARCS uses Protocol Buffers (protobuf) for message definitions. This provides:
- Language-neutral serialization
- Version compatibility
- Compact binary format
- Strong typing
- Auto-generated code

## Message Categories

### 1. Common Messages (`common.proto`)

#### Time
Represents a point in time with nanosecond precision.
```protobuf
message Time {
  int64 sec = 1;        // Seconds since epoch
  uint32 nanosec = 2;   // Nanoseconds component [0, 999999999]
}
```

**Usage**: Timestamps, duration measurements, time synchronization

#### Header
Standard header included in most messages.
```protobuf
message Header {
  Time timestamp = 1;    // When message was created
  string frame_id = 2;   // Reference frame (e.g., "base_link", "odom")
  uint64 seq = 3;        // Sequence number for ordering
}
```

**Usage**: All time-stamped messages (sensor data, commands, telemetry)

### 2. Geometry Messages (`geometry.proto`)

#### Vector3
3D vector for positions, velocities, forces.
```protobuf
message Vector3 {
  double x = 1;
  double y = 2;
  double z = 3;
}
```

**Units**: Context-dependent (typically meters, m/s, or N)

#### Quaternion
Orientation representation (normalized).
```protobuf
message Quaternion {
  double x = 1;
  double y = 2;
  double z = 3;
  double w = 4;
}
```

**Constraint**: Should be normalized (x² + y² + z² + w² = 1)

#### Pose
Complete position and orientation.
```protobuf
message Pose {
  Vector3 position = 1;
  Quaternion orientation = 2;
}
```

**Usage**: Robot pose, target waypoints, object locations

#### Twist
Linear and angular velocity.
```protobuf
message Twist {
  Vector3 linear = 1;    // Linear velocity [m/s]
  Vector3 angular = 2;   // Angular velocity [rad/s]
}
```

**Usage**: Velocity commands, odometry

### 3. Control Messages (`control.proto`)

#### TrajectoryPoint
Single point in a trajectory.
```protobuf
message TrajectoryPoint {
  double t = 1;           // Time from start [seconds]
  Pose pose = 2;          // Target pose at this time
  Twist velocity = 3;     // Target velocity at this time
}
```

#### TrajectoryPlan
Complete trajectory for robot to execute.
```protobuf
message TrajectoryPlan {
  Header header = 1;
  string plan_id = 2;                     // Unique identifier
  repeated TrajectoryPoint points = 3;    // Trajectory waypoints
  double v_max = 4;                       // Max linear velocity [m/s]
  double a_max = 5;                       // Max linear accel [m/s²]
  double omega_max = 6;                   // Max angular velocity [rad/s]
}
```

**Usage**: High-level motion commands from planner to controller

#### WheelState
Current state of drive wheels.
```protobuf
message WheelState {
  Header header = 1;
  repeated double rpm = 2;           // RPM for each wheel
  repeated int64 encoder_ticks = 3;  // Raw encoder values
}
```

**Usage**: Odometry, motor control feedback, diagnostics

### 4. Health Messages (`health.proto`)

#### ComponentStatus
Status of a single system component.
```protobuf
message ComponentStatus {
  enum State {
    UNKNOWN = 0;
    OK = 1;
    WARNING = 2;
    ERROR = 3;
    FATAL = 4;
  }
  
  string name = 1;      // Component identifier
  State state = 2;      // Current health state
  string message = 3;   // Human-readable status
  double value = 4;     // Optional metric (temp, load, etc.)
}
```

#### SystemHealth
Overall system health report.
```protobuf
message SystemHealth {
  Header header = 1;
  repeated ComponentStatus components = 2;
}
```

**Usage**: Monitoring, diagnostics, automated fault detection

## Design Principles

### 1. Extensibility
- Messages can add new fields without breaking compatibility
- Optional fields for backward compatibility
- Reserved field numbers for deprecated fields

### 2. Consistency
- All stamped messages include `Header`
- All timestamps use `Time` type
- All poses use `Pose` (position + orientation)

### 3. Efficiency
- Compact binary format
- Zero-copy deserialization possible
- Repeated fields for arrays

### 4. Interoperability
- Language-independent definitions
- Can interface with ROS via bridge
- Easy to generate for Python, Java, etc.

## Message Usage Patterns

### Publishing Pattern
```cpp
// Create message
TrajectoryPlan plan;
plan.mutable_header()->set_timestamp(...);
plan.set_plan_id("traj_001");
// ... fill in details

// Publish
publisher.Publish(plan);
```

### Subscription Pattern
```cpp
void OnTrajectory(const TrajectoryPlan& plan) {
  // Process received plan
  for (const auto& point : plan.points()) {
    // Execute trajectory point
  }
}

Subscriber<TrajectoryPlan> sub("trajectory", OnTrajectory);
```

## Extension Guidelines

### Adding New Message Types
1. Choose appropriate `.proto` file (or create new one)
2. Define message with clear field names
3. Document purpose and units in comments
4. Add to `msgs/CMakeLists.txt` if new file
5. Update this documentation

### Versioning
- Use semantic versioning for proto packages
- Add new optional fields instead of modifying existing
- Use reserved field numbers for removed fields
- Document breaking changes

### Best Practices
- Keep messages focused and cohesive
- Use nested messages for complex structures
- Document units in comments (m, rad, s, etc.)
- Use enums for fixed sets of values
- Include Header for time-critical data
