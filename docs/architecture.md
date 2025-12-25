# LARCS Architecture

## Overview

LARCS (Autonomous Robot Competition System) is a modular framework designed for developing, testing, and deploying autonomous robot systems for competitive environments. The system provides a ROS-independent architecture that enables seamless transition from simulation to real hardware.

## System Components

### High-Level Architecture

```
┌─────────────┐      ┌─────────────┐      ┌─────────────┐
│   Simulation│      │    Real     │      │    Server   │
│    System   │◄────►│   Hardware  │◄────►│   Control   │
└─────────────┘      └─────────────┘      └─────────────┘
       ▲                    ▲                     ▲
       │                    │                     │
       └────────────────────┴─────────────────────┘
                     LARCS Runtime
```

### Core Modules

#### 1. **larcs-runtime** (Communication & Time)
The runtime module provides fundamental capabilities:
- **Time Management**: Monotonic and system time handling
- **Publisher/Subscriber**: Message-passing infrastructure (skeleton)
- **Logger**: Structured logging with spdlog integration
- **Future**: Network transport, serialization, node lifecycle

#### 2. **larcs-msgs** (Message Definitions)
Protocol buffer definitions for all communication:
- **common**: Time, Header
- **geometry**: Vector3, Quaternion, Pose, Twist
- **control**: TrajectoryPlan, WheelState
- **health**: SystemHealth, ComponentStatus

#### 3. **larcs-tools** (Command-Line Utilities)
- **larcs-ping**: Network connectivity testing
- **larcs-record**: Message recording (skeleton)
- **larcs-replay**: Message playback (skeleton)

## Communication Design

### Message Flow

```
Control Station         Robot System
┌─────────────┐        ┌─────────────┐
│  Trajectory │───────►│   Control   │
│   Planner   │        │  Executor   │
└─────────────┘        └─────────────┘
       ▲                      │
       │                      ▼
┌─────────────┐        ┌─────────────┐
│  Telemetry  │◄───────│   Sensors   │
│   Monitor   │        │  & Wheels   │
└─────────────┘        └─────────────┘
```

### Communication Channels

1. **Control Channel**: High-frequency (50-100Hz) trajectory commands
2. **Telemetry Channel**: Sensor data, wheel states, odometry
3. **Health Channel**: System status monitoring (1-10Hz)
4. **Perception Channel**: Vision, LIDAR data (variable rate)

## Control Cycle Design

### Typical Control Loop (100Hz)
```
1. Receive trajectory plan (TrajectoryPlan)
2. Interpolate current target pose
3. Compute wheel velocities
4. Send commands to motor controllers
5. Read encoder feedback (WheelState)
6. Update odometry
7. Publish telemetry
```

### Time Synchronization
- All messages include `Header` with timestamp
- System uses monotonic time for control loops
- Wall clock time for logging and diagnostics

## Simulation to Real Transfer

### Development Workflow

```
Phase 1: Simulation Development
  ├─ Develop algorithms in simulation
  ├─ Test with virtual sensors/actuators
  └─ Record test scenarios

Phase 2: Hardware Integration
  ├─ Replace simulation components with hardware drivers
  ├─ Maintain same message interfaces
  └─ Replay recorded scenarios on real hardware

Phase 3: Field Testing
  ├─ Deploy to competition robot
  ├─ Monitor via telemetry
  └─ Record real-world data for analysis
```

### Key Principles
1. **Interface Consistency**: Same APIs for sim and real
2. **Message-Level Compatibility**: Exact protobuf definitions
3. **Time Abstraction**: Runtime handles timing details
4. **Modular Components**: Swap implementations without code changes

## Module Dependencies

```
tools/          → runtime, msgs
runtime/        → (standalone, minimal deps)
msgs/           → protobuf only
```

## Future Expansion

### Planned Features (Rev.1+)
- Network transport layer (UDP/TCP)
- Distributed node architecture
- Real-time scheduling support
- Hardware abstraction layer
- Simulation integration layer
- Competition-specific modules:
  - Path planning
  - Localization
  - Object detection
  - Strategy executor

### Extension Points
- Custom message types via protobuf
- Pluggable transport mechanisms
- Custom time sources (e.g., simulation time)
- Logger backends (database, cloud)

## Design Goals

1. **Minimal Dependencies**: Keep core runtime lean
2. **ROS Independence**: No ROS required, but interoperable
3. **CLion Integration**: Seamless IDE experience
4. **Cross-Platform**: Linux primary, others possible
5. **Performance**: Hard real-time capable where needed
6. **Maintainability**: Clear structure, good documentation
