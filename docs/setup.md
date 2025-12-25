# LARCS Development Environment Setup

This guide covers setting up a development environment for LARCS Engine on Ubuntu 24.04 LTS.

## Prerequisites

### System Requirements
- **OS**: Ubuntu 24.04 LTS
- **RAM**: 4GB minimum, 8GB recommended
- **Disk**: 10GB free space
- **Compiler**: GCC 13+ or Clang 17+

## Installation Steps

### 1. System Packages

Update your system and install essential build tools:

```bash
sudo apt update
sudo apt upgrade -y
sudo apt install -y \
    build-essential \
    cmake \
    ninja-build \
    git \
    curl \
    zip \
    unzip \
    tar \
    pkg-config \
    clang-format \
    clang-tidy
```

### 2. vcpkg Installation

Install vcpkg for dependency management:

```bash
# Clone vcpkg
cd ~
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg

# Bootstrap vcpkg
./bootstrap-vcpkg.sh

# Add to PATH (add to ~/.bashrc for persistence)
export VCPKG_ROOT=~/vcpkg
export PATH=$VCPKG_ROOT:$PATH

# Optional: Add to .bashrc
echo 'export VCPKG_ROOT=~/vcpkg' >> ~/.bashrc
echo 'export PATH=$VCPKG_ROOT:$PATH' >> ~/.bashrc
```

### 3. Clone LARCS Repository

```bash
cd ~/projects  # or your preferred location
git clone https://github.com/dev-Earth/LARCS_Engine.git
cd LARCS_Engine
```

### 4. Build the Project

#### Using CMake Presets (Recommended)

```bash
# Configure with default preset
cmake --preset default

# Build
cmake --build build/default

# Run tests
cd build/default
ctest --output-on-failure
```

#### Using Debug Preset

```bash
# Configure debug build
cmake --preset debug

# Build
cmake --build build/debug

# Run tests with verbose output
cd build/debug
ctest --output-on-failure -V
```

### 5. Verify Installation

```bash
# Test tools
./build/default/tools/larcs-ping --help
./build/default/tools/larcs-record --help
./build/default/tools/larcs-replay --help

# Run tests
cd build/default
ctest
```

Expected output:
```
Test project .../LARCS_Engine/build/default
    Start 1: time_test
1/1 Test #1: time_test ........................   Passed    0.01 sec

100% tests passed, 0 tests failed out of 1
```

## CLion IDE Setup

### 1. Install CLion

Download and install CLion from [JetBrains website](https://www.jetbrains.com/clion/).

### 2. Open Project

1. Launch CLion
2. Select "Open" and navigate to the LARCS_Engine directory
3. CLion should automatically detect `CMakeLists.txt`

### 3. Configure CMake in CLion

CLion will automatically detect the CMake presets defined in `CMakePresets.json`:

1. Go to **File → Settings → Build, Execution, Deployment → CMake**
2. You should see profiles for:
   - `clion` (Release)
   - `clion-debug` (Debug)
3. Select your preferred profile (or use both)

### 4. Set Environment Variables

If CLion doesn't find vcpkg:

1. Go to **File → Settings → Build, Execution, Deployment → CMake**
2. In the CMake profile, expand "Environment"
3. Add: `VCPKG_ROOT=/home/your_username/vcpkg`

### 5. Build in CLion

- Use **Build → Build Project** or press `Ctrl+F9`
- Select targets from the dropdown in the toolbar

### 6. Run Tests in CLion

1. Open the "Run" toolbar dropdown
2. Select any test (e.g., `time_test`)
3. Click the green run arrow or press `Shift+F10`

### 7. Code Formatting

CLion will use the `.clang-format` file automatically:

- Format file: `Ctrl+Alt+L`
- Format on save: **Settings → Tools → Actions on Save → Reformat code**

## Development Workflow

### Building

```bash
# Full rebuild
cmake --build build/default --clean-first

# Parallel build (faster)
cmake --build build/default -j$(nproc)

# Build specific target
cmake --build build/default --target larcs-ping
```

### Testing

```bash
# Run all tests
cd build/default
ctest

# Run specific test
./runtime/time_test

# Verbose test output
ctest -V

# Run tests matching pattern
ctest -R time
```

### Code Quality

```bash
# Format all code
find . -name "*.cpp" -o -name "*.hpp" | xargs clang-format -i

# Run clang-tidy (from build directory)
cd build/default
run-clang-tidy
```

## Troubleshooting

### vcpkg Issues

**Problem**: CMake can't find vcpkg
```bash
# Ensure VCPKG_ROOT is set
echo $VCPKG_ROOT

# Should output: /home/your_username/vcpkg
# If not, export it:
export VCPKG_ROOT=~/vcpkg
```

**Problem**: vcpkg fails to install packages
```bash
# Clear vcpkg cache
rm -rf $VCPKG_ROOT/buildtrees
rm -rf $VCPKG_ROOT/downloads

# Try installing manually
cd $VCPKG_ROOT
./vcpkg install fmt spdlog protobuf cli11 gtest
```

### Build Issues

**Problem**: Protobuf generation fails
```bash
# Ensure protobuf is installed
vcpkg list | grep protobuf

# Manually install if needed
vcpkg install protobuf
```

**Problem**: GTest not found
```bash
# Install GTest via vcpkg
vcpkg install gtest
```

### CLion Issues

**Problem**: CLion shows "CMake Error"
- Check that VCPKG_ROOT is set in CLion settings
- Try "File → Reload CMake Project"
- Invalidate caches: "File → Invalidate Caches → Invalidate and Restart"

**Problem**: Code completion not working
- Ensure "Rescan files" has completed (bottom right of CLion)
- Regenerate CMake: "Tools → CMake → Reset Cache and Reload Project"

## Next Steps

- Read [architecture.md](architecture.md) for system overview
- Read [messaging.md](messaging.md) for message definitions
- Explore example code in `runtime/tests/`
- Start developing your robot control logic!

## Additional Resources

- [CMake Documentation](https://cmake.org/documentation/)
- [vcpkg Documentation](https://vcpkg.io/)
- [Protocol Buffers Guide](https://protobuf.dev/)
- [CLion Documentation](https://www.jetbrains.com/clion/learn/)
