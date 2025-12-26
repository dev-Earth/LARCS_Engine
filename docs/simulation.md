# LARCS Matrix - Physics Simulation Engine

## Overview

LARCS Matrix is the physics simulation component of the LARCS Engine, providing a complete Digital Twin environment for autonomous robot development. Built on NVIDIA PhysX 5.x, it enables seamless Sim-to-Real transfer by accurately modeling robot dynamics, sensors, and environments.

## Key Features

- **High-Fidelity Physics**: PhysX 5.x for accurate rigid body dynamics
- **Fixed Time Step**: Deterministic 240Hz simulation for reproducibility
- **Sensor Framework**: Modular sensor system with realistic noise models
- **Time Scaling**: Run simulations at 1x (real-time) or faster (10x, 100x)
- **Headless Mode**: Run without visualization for CI/CD and batch processing
- **Zenoh Integration**: Same communication layer as real robots

## Architecture

### Digital Twin Philosophy

LARCS Matrix implements a true Digital Twin:
- **Same Code**: Control algorithms run identically in sim and real
- **Same Communication**: Zenoh pub/sub works the same way
- **Same Sensors**: Sensor data format matches real hardware
- **Same Timing**: Real-time synchronization ensures timing accuracy

### Core Components

1. **World**: PhysX scene manager with gravity and time control
2. **Robot Assembly**: Container for robot body, wheels, and sensors
3. **Sensor Framework**: Pluggable sensor models (encoders, IMU, etc.)
4. **USD Loader**: Import robot models from CAD (Phase 2.2+)

## PhysX 5.x Benefits

### Why PhysX?

- **Industry Standard**: Used in games, film, and robotics
- **High Performance**: GPU acceleration for complex scenes
- **Stability**: Proven in production environments
- **NVIDIA Ecosystem**: Integrates with Omniverse, Isaac Sim

### Fixed vs Variable Time Step

LARCS Matrix uses a **fixed time step** (1/240s = 4.167ms):

**Benefits:**
- Deterministic behavior (same inputs → same outputs)
- Easier debugging and reproduction
- Stable physics simulation
- Predictable sensor timing

**Trade-offs:**
- Must maintain 240Hz for real-time (achievable for most robots)
- Slower than real-time if simulation is too complex

## Time Scaling

### Real-Time Mode (1.0x)

```bash
larcs-matrix --config robot.yaml --speed 1.0
```

Synchronizes simulation time with wall clock time:
- Good for testing with real hardware
- Useful for human observation
- Matches real robot behavior exactly

### Fast Mode (10x, 100x)

```bash
larcs-matrix --config robot.yaml --speed 10.0
```

Runs simulation faster than real-time:
- Rapid testing and iteration
- Batch experiments
- Training reinforcement learning agents
- No visualization delay

## Headless Execution

Run without graphics for automated testing:

```bash
larcs-matrix --config robot.yaml --headless
```

Benefits:
- Lower CPU/memory usage
- Suitable for CI/CD pipelines
- Batch processing on servers
- No display required

## Simulation Loop

```
Initialize:
  - Create PhysX world
  - Load robot from config
  - Spawn robot in scene
  - Connect to Zenoh

Main Loop (240 Hz):
  - Read cmd_vel commands
  - Update robot kinematics
  - Step PhysX simulation
  - Update sensors
  - Publish sensor data
  - Sync with real-time clock

Shutdown:
  - Clean up PhysX
  - Close Zenoh connection
```

## Configuration

Simulation parameters are controlled via YAML config files:

```yaml
simulation:
  physics:
    timestep: 0.004167  # 1/240 s
    gravity: -9.81      # m/s^2
```

See `configs/sim_default.yaml` for a complete example.

## Performance Considerations

### Real-Time Constraints

To maintain 240Hz real-time:
- Keep robot complexity reasonable (<100 bodies)
- Use simplified collision geometry
- Avoid expensive sensors (LiDAR raycast) at high rates

### Fast Simulation

For >10x speed:
- Reduce sensor update rates
- Disable unnecessary features
- Run headless
- Use CPU efficiently

## Comparison with Other Simulators

| Feature | LARCS Matrix | Gazebo | Isaac Sim | Webots |
|---------|-------------|--------|-----------|--------|
| Physics | PhysX 5.x | ODE/Bullet | PhysX 5.x | ODE |
| Fixed Step | ✅ 240Hz | ❌ Variable | ✅ Configurable | ✅ Configurable |
| Time Scale | ✅ | ✅ | ✅ | ✅ |
| Headless | ✅ | ✅ | ✅ | ✅ |
| Lightweight | ✅ | ❌ Heavy | ❌ Heavy | ✅ |
| ROS Free | ✅ | ❌ ROS Required | ❌ ROS Optional | ✅ |

## Usage Examples

### Basic Simulation

```bash
# Start simulation with default config
larcs-matrix --config configs/sim_default.yaml

# In another terminal, control the robot
larcs-pub /robot/cmd_vel '{"linear":{"x":1.0},"angular":{"z":0.5}}' -t Twist
```

### Fast Testing

```bash
# Run 10x faster than real-time
larcs-matrix --config configs/sim_default.yaml --speed 10.0
```

### CI/CD Testing

```bash
# Headless mode for automated tests
larcs-matrix --config configs/test_robot.yaml --headless &
sleep 2
./run_tests.sh
pkill larcs-matrix
```

## Future Enhancements

### Phase 2.2
- Full USD integration with physics properties
- LiDAR sensor with raycast
- Depth camera sensor
- GPU acceleration

### Phase 2.3
- Fusion360 plugin for automatic export
- Visual debugging tools
- Multi-robot simulation
- Integration with Omniverse

## Troubleshooting

### Simulation Runs Too Slow

- Reduce sensor update rates
- Simplify collision geometry
- Use headless mode
- Check CPU usage

### Physics Looks Wrong

- Verify mass and inertia values
- Check friction coefficients
- Ensure proper joint configuration
- Validate timestep (should be 1/240)

### Zenoh Not Connecting

- Check if zenohd is running
- Verify network configuration
- Test with larcs-ping
- Check firewall settings

## References

- [PhysX Documentation](https://nvidia-omniverse.github.io/PhysX/physx/5.4.0/)
- [USD Documentation](https://graphics.pixar.com/usd/docs/index.html)
- [Zenoh Documentation](https://zenoh.io/docs/)
