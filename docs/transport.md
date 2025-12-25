# LARCS Transport Layer

## Overview

LARCS uses **Zenoh** as its communication transport layer. Zenoh provides a high-performance, ROS-independent pub/sub and RPC system with automatic peer discovery, making it ideal for robotics applications that need to work across simulation, real hardware, and monitoring tools.

## What is Zenoh?

[Zenoh](https://zenoh.io/) is a modern communication middleware designed for:

- **High Performance**: Zero-copy transfers, low latency
- **Auto Discovery**: Peers automatically find each other via multicast scouting
- **Flexible Deployment**: Works across processes, machines, and networks
- **ROS Independence**: No dependency on ROS infrastructure
- **Unified API**: Same code works for local and remote communication

Unlike traditional middleware, Zenoh is designed from the ground up for real-time robotics with minimal overhead.

## Architecture

### Transport Abstraction

LARCS provides a clean abstraction layer over Zenoh:

```cpp
// Initialize transport
auto transport = std::make_shared<larcs::runtime::ZenohTransport>();
transport->initialize();

// Create publisher
auto pub = std::make_shared<larcs::runtime::Publisher<larcs::msgs::Twist>>(
    transport, "/cmd_vel", QoSProfile::Control);

// Create subscriber
auto sub = std::make_shared<larcs::runtime::Subscriber<larcs::msgs::Twist>>(
    transport, "/cmd_vel", 
    [](const larcs::msgs::Twist& msg) {
        // Handle received message
    },
    QoSProfile::Control);

// Publish message
larcs::msgs::Twist twist;
twist.mutable_linear()->set_x(1.0);
pub->publish(twist);
```

### QoS Profiles

LARCS defines three Quality-of-Service profiles optimized for different use cases:

#### Control (High Reliability)
- **Use Case**: Trajectory commands, control signals, emergency stop
- **Congestion Control**: BLOCK (wait for capacity)
- **Priority**: REAL_TIME (highest)
- **Reliability**: Messages are not dropped

```cpp
QoSProfile::Control
```

#### Telemetry (Latest Value)
- **Use Case**: Robot state, odometry, sensor readings
- **Congestion Control**: DROP (discard old data)
- **Priority**: DATA (normal)
- **Reliability**: Latest value is most important

```cpp
QoSProfile::Telemetry
```

#### Perception (High Throughput)
- **Use Case**: Point clouds, images, large sensor data
- **Congestion Control**: DROP (discard when congested)
- **Priority**: DATA_LOW (background)
- **Reliability**: Best effort

```cpp
QoSProfile::Perception
```

## Network Configuration

### Peer Mode

LARCS uses Zenoh in **peer mode** by default, which means:
- All nodes are equal (no central broker required)
- Automatic peer discovery via multicast
- Scales from single machine to distributed systems

### Multicast Scouting

Zenoh peers discover each other automatically using UDP multicast on the local network:
- **Multicast Address**: 224.0.0.224:7447 (default)
- **Discovery**: Automatic, zero configuration
- **Works**: Same machine, same LAN, same subnet

## Usage

### C++ API

#### Publisher Example

```cpp
#include "larcs/runtime/publisher.hpp"
#include "larcs/runtime/zenoh_transport.hpp"
#include "larcs/msgs/geometry.pb.h"

auto transport = std::make_shared<larcs::runtime::ZenohTransport>();
transport->initialize();

auto pub = std::make_shared<larcs::runtime::Publisher<larcs::msgs::Twist>>(
    transport, "/robot/cmd_vel", larcs::runtime::QoSProfile::Control);

larcs::msgs::Twist cmd;
cmd.mutable_linear()->set_x(1.5);
cmd.mutable_angular()->set_z(0.3);

pub->publish(cmd);
```

#### Subscriber Example

```cpp
#include "larcs/runtime/subscriber.hpp"
#include "larcs/runtime/zenoh_transport.hpp"
#include "larcs/msgs/geometry.pb.h"

auto transport = std::make_shared<larcs::runtime::ZenohTransport>();
transport->initialize();

auto sub = std::make_shared<larcs::runtime::Subscriber<larcs::msgs::Twist>>(
    transport, "/robot/cmd_vel",
    [](const larcs::msgs::Twist& msg) {
        std::cout << "Received: linear.x = " << msg.linear().x() << std::endl;
    },
    larcs::runtime::QoSProfile::Control);

// Keep running to receive messages
while (true) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}
```

### CLI Tools

LARCS provides command-line tools for testing and debugging:

#### larcs-pub

Publish a message to a topic:

```bash
# Publish Twist message
./build/default/tools/larcs-pub /cmd_vel '{"linear":{"x":1.0},"angular":{"z":0.5}}' -t Twist

# Publish Pose message
./build/default/tools/larcs-pub /pose '{"position":{"x":1.0,"y":2.0,"z":0.0}}' -t Pose

# Publish Vector3 message
./build/default/tools/larcs-pub /point '{"x":1.0,"y":2.0,"z":3.0}' -t Vector3
```

#### larcs-sub

Subscribe to messages from a topic:

```bash
# Subscribe to Twist messages (unlimited)
./build/default/tools/larcs-sub /cmd_vel -t Twist

# Subscribe and exit after 10 messages
./build/default/tools/larcs-sub /cmd_vel -t Twist --count 10

# Subscribe to Pose messages with verbose output
./build/default/tools/larcs-sub /pose -t Pose -v
```

## Testing Communication

### Single Machine Test

In one terminal:
```bash
./build/default/tools/larcs-sub /test/twist -t Twist
```

In another terminal:
```bash
./build/default/tools/larcs-pub /test/twist '{"linear":{"x":1.5},"angular":{"z":0.5}}' -t Twist
```

You should see the message printed in the subscriber terminal.

### Multi-Process Test

The same test works across different processes automatically thanks to Zenoh's automatic discovery.

### Multi-Machine Test

On Machine A:
```bash
./build/default/tools/larcs-sub /robot/cmd_vel -t Twist
```

On Machine B (same network):
```bash
./build/default/tools/larcs-pub /robot/cmd_vel '{"linear":{"x":1.0}}' -t Twist
```

Messages are automatically routed between machines.

## Troubleshooting

### No Messages Received

1. **Check Firewall**: Ensure UDP multicast is allowed
   ```bash
   # Ubuntu/Debian
   sudo ufw allow proto udp to 224.0.0.0/4
   ```

2. **Check Multicast Routes**: Verify multicast routing is enabled
   ```bash
   ip route show | grep 224
   # Should show: 224.0.0.0/4 dev <interface> ...
   ```

3. **Add Multicast Route** (if missing):
   ```bash
   sudo ip route add 224.0.0.0/4 dev eth0  # or your network interface
   ```

### High Latency

- Use `QoSProfile::Control` for time-critical messages
- Check network congestion
- Ensure sufficient CPU resources

### Messages Dropped

- Check if `QoSProfile::Telemetry` or `Perception` is appropriate
- For critical messages, use `QoSProfile::Control`
- Monitor network bandwidth

### Discovery Issues

1. **Same Subnet**: Ensure machines are on the same subnet
2. **VPN Issues**: Some VPNs block multicast
3. **Docker/VM**: May need special network configuration

### Enable Debug Logging

```cpp
#include <spdlog/spdlog.h>
spdlog::set_level(spdlog::level::debug);
```

Or use the `-v` flag with CLI tools:
```bash
./build/default/tools/larcs-sub /topic -t Twist -v
```

## Performance Characteristics

### Latency
- Local (same process): < 1 μs
- Inter-process (same machine): < 100 μs
- Network (LAN): < 1 ms

### Throughput
- Small messages (< 1 KB): > 100k msg/s
- Large messages (> 1 MB): Limited by network bandwidth

### Discovery Time
- Local network: < 100 ms
- Cross-subnet: Depends on router configuration

## Integration with LARCS

### Simulation Mode
Transport works identically in simulation and real modes.

### Real Robot Mode
Same API, zero code changes needed.

### Monitor Mode
CLI tools and custom monitors can observe all traffic.

## Future Extensions

Planned for future phases:
- **Service/Client RPC**: Request-response patterns
- **Recorder/Replayer**: Integration with MCAP format
- **Network Statistics**: Latency, loss, bandwidth monitoring
- **Advanced Routing**: Custom routing policies

## References

- [Zenoh Documentation](https://zenoh.io/docs/)
- [Zenoh GitHub](https://github.com/eclipse-zenoh/zenoh)
- [LARCS Architecture](architecture.md)
- [Message Specifications](messaging.md)
