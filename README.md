# **Learning Autonomous Robot Control System Engine**

### 概要
#### LARCS（Learning Autonomous Robot Control System Engine）は、自律型ロボットの制御システムを学習・開発するための統合3Dエンジンです。LARCSは、ロボットの動作をシミュレーションし、強化学習アルゴリズムを用いて最適な制御戦略を学習することができます。

### 主な特徴
- **3Dシミュレーション環境**: 高度な3Dグラフィックスと物理エンジンを備えたシミュレーション環境を提供します。
    - **強化学習アルゴリズム**: Q学習、Deep Q-Networks（DQN）、Proximal Policy Optimization（PPO）など、様々な強化学習アルゴリズムをサポートしています。
    - **モジュール式設計**: ロボットのセンサー、アクチュエーター、制御アルゴリズムを簡単に追加・交換できるモジュール式設計を採用しています。
    - **リアルタイムデータ解析**: ロボットの動作データをリアルタイムで解析し、学習プロセスを可視化します。

## Overview

LARCS Engine provides a lightweight, high-performance foundation for building autonomous robots, with a focus on competitive robotics environments. The system emphasizes:

- **Seamless Sim-to-Real Transfer**: Develop in simulation, deploy to hardware with minimal changes
- **Minimal Dependencies**: Lean core runtime without ROS overhead
- **Modern C++**: C++20 features with strong typing and performance
- **IDE-Friendly**: Full CLion integration with vcpkg package management
- **Extensible Architecture**: Modular design for easy customization

## Features

### Current (Rev.1)
- ✅ Time management (monotonic and system time)
- ✅ Structured logging with spdlog
- ✅ Protocol buffer message definitions
- ✅ Zenoh-based pub/sub transport
- ✅ Publisher/Subscriber with QoS profiles
- ✅ CLI tools (pub, sub, ping, record, replay)
- ✅ CMake preset integration
- ✅ Unit testing framework
- ✅ Auto-discovery across processes and machines

### Planned (Future Revisions)
- Service/Client RPC patterns
- Message recording/replay with MCAP
- Network statistics and monitoring
- Hardware abstraction layer
- Path planning and navigation
- Sensor fusion and localization
- Competition-specific modules

## Quick Start

### Prerequisites
- Ubuntu 24.04 LTS (or similar Linux distribution)
- GCC 13+ or Clang 17+
- CMake 3.25+
- vcpkg (installed below)

### Installation

1. **Install System Dependencies**
```bash
sudo apt update
sudo apt install -y build-essential cmake ninja-build git curl zip unzip tar
```

2. **Install vcpkg**
```bash
cd ~
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh
export VCPKG_ROOT=~/vcpkg
export PATH=$VCPKG_ROOT:$PATH
```

3. **Clone and Build**
```bash
git clone https://github.com/dev-Earth/LARCS_Engine.git
cd LARCS_Engine
cmake --preset default
cmake --build build/default
```

4. **Run Tests**
```bash
cd build/default
ctest --output-on-failure
```

### Usage

After building, try the tools:

```bash
# Pub/Sub communication
./build/default/tools/larcs-pub --help
./build/default/tools/larcs-sub --help

# Example: Publish and subscribe to messages
# Terminal 1:
./build/default/tools/larcs-sub /test/twist -t Twist

# Terminal 2:
./build/default/tools/larcs-pub /test/twist '{"linear":{"x":1.5},"angular":{"z":0.5}}' -t Twist

# Network connectivity test
./build/default/tools/larcs-ping --help
./build/default/tools/larcs-ping -h 127.0.0.1 -p 8888 -c 4

# Recording tool (skeleton)
./build/default/tools/larcs-record -o data.log -t /odom /cmd_vel

# Replay tool (skeleton)
./build/default/tools/larcs-replay -i data.log -r 1.0
```

## Project Structure

```
LARCS_Engine/
├── cmake/                  # CMake helper functions
│   └── larcs.cmake
├── docs/                   # Documentation
│   ├── architecture.md     # System design overview
│   ├── messaging.md        # Message specifications
│   └── setup.md           # Detailed setup guide
├── msgs/                   # Protocol buffer definitions
│   ├── proto/
│   │   ├── common.proto    # Time, Header
│   │   ├── geometry.proto  # Pose, Twist, etc.
│   │   ├── control.proto   # Trajectory, WheelState
│   │   └── health.proto    # System health monitoring
│   └── CMakeLists.txt
├── runtime/                # Core runtime library
│   ├── include/larcs/runtime/
│   │   ├── time.hpp        # Time utilities
│   │   ├── publisher.hpp   # Message publisher
│   │   ├── subscriber.hpp  # Message subscriber
│   │   └── logger.hpp      # Logging system
│   ├── src/                # Implementation
│   ├── tests/              # Unit tests
│   └── CMakeLists.txt
├── tools/                  # Command-line utilities
│   ├── src/
│   │   ├── ping.cpp        # Network ping tool
│   │   ├── record.cpp      # Recording tool (skeleton)
│   │   └── replay.cpp      # Replay tool (skeleton)
│   └── CMakeLists.txt
├── .clang-format          # Code formatting rules
├── .clang-tidy            # Static analysis config
├── CMakeLists.txt         # Root build configuration
├── CMakePresets.json      # CMake presets for CLion
└── vcpkg.json            # Dependency manifest
```

## Dependencies

Managed via vcpkg:
- **fmt**: String formatting
- **spdlog**: Fast logging library
- **protobuf**: Message serialization
- **CLI11**: Command-line parsing
- **gtest**: Testing framework
- **zenoh-c**: High-performance pub/sub communication

## Communication Layer

LARCS uses [Zenoh](https://zenoh.io/) for inter-process and network communication:
- **Auto Discovery**: Zero configuration peer discovery
- **High Performance**: Sub-millisecond latency
- **ROS Independent**: Works without ROS infrastructure
- **Flexible**: Same code for local and distributed systems

See [docs/transport.md](docs/transport.md) for detailed documentation.

### Quick Test

After building, test the pub/sub system:

```bash
# Terminal 1: Subscribe
./build/default/tools/larcs-sub /test/twist -t Twist

# Terminal 2: Publish
./build/default/tools/larcs-pub /test/twist '{"linear":{"x":1.5},"angular":{"z":0.5}}' -t Twist
```

## CLion Setup

LARCS is designed for seamless CLion integration:

1. Open the project in CLion
2. CLion will automatically detect CMake presets
3. Select `clion` or `clion-debug` profile
4. Build and run!

See [docs/setup.md](docs/setup.md) for detailed CLion configuration.

## Building

### Using CMake Presets

```bash
# Release build
cmake --preset default
cmake --build build/default

# Debug build
cmake --preset debug
cmake --build build/debug

# CLion-compatible build
cmake --preset clion
cmake --build cmake-build-release
```

### Manual CMake

```bash
mkdir build && cd build
cmake -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake \
  ..
cmake --build .
```

## Testing

```bash
# Run all tests
cd build/default
ctest

# Verbose output
ctest --output-on-failure -V

# Run specific test
./runtime/time_test
```

## Development

### Code Formatting

The project uses Google C++ Style (with minor tweaks):

```bash
# Format a file
clang-format -i path/to/file.cpp

# Format all files
find . -name "*.cpp" -o -name "*.hpp" | xargs clang-format -i
```

### Static Analysis

```bash
# From build directory
run-clang-tidy
```

## Documentation

- [Architecture Overview](docs/architecture.md) - System design and components
- [Message Specifications](docs/messaging.md) - Protocol buffer message details
- [Transport Layer](docs/transport.md) - Zenoh communication system
- [Setup Guide](docs/setup.md) - Detailed installation and configuration

## Roadmap

### Rev.0 - Foundation ✅
- ✅ Basic project structure
- ✅ Build system with vcpkg
- ✅ Time and logging utilities
- ✅ Message definitions
- ✅ CLI tools skeleton

### Rev.1 - Communication (Current) ✅
- ✅ Zenoh transport implementation
- ✅ Complete pub/sub system with QoS
- ✅ CLI pub/sub tools
- ✅ Multi-node auto-discovery
- ⏳ Message recording/replay (planned)

### Rev.2 - Robot Control
- Trajectory execution
- Wheel controller interface
- Odometry integration
- Hardware abstraction layer

### Rev.3 - Autonomy
- Path planning
- Localization
- Sensor integration
- Competition logic

### Rev.4 - Advanced Features
- Multi-robot coordination
- Advanced diagnostics
- Performance optimization
- Competition-ready system

## Contributing

This is a competition project for the LARCS autonomous robot competition. While the codebase is public, active development is managed by the team.

## License

[To be determined - License file not yet added]

## Contact

For questions or collaboration, open an issue on GitHub.

---

**LARCS Engine** - Built for speed, designed for competition 🏁
