#!/bin/bash
# PhysX 5.x Installation Script for LARCS Engine
# https://github.com/NVIDIA-Omniverse/PhysX

set -e

PHYSX_VERSION="5.4.1"
PHYSX_REPO="https://github.com/NVIDIA-Omniverse/PhysX.git"
INSTALL_DIR="/usr/local"
BUILD_DIR="/tmp/physx-build"

echo "Installing PhysX ${PHYSX_VERSION}..."

# Install dependencies
echo "Installing build dependencies..."
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    git \
    python3 \
    libgl1-mesa-dev \
    libglu1-mesa-dev \
    libx11-dev \
    libxrandr-dev \
    libxinerama-dev \
    libxcursor-dev \
    libxi-dev

# Clone PhysX repository
if [ -d "$BUILD_DIR" ]; then
    echo "Removing existing build directory..."
    rm -rf "$BUILD_DIR"
fi

echo "Cloning PhysX repository..."
git clone --depth 1 --branch "$PHYSX_VERSION" "$PHYSX_REPO" "$BUILD_DIR"

cd "$BUILD_DIR/physx"

# Generate project files
echo "Generating build files..."
./generate_projects.sh linux

# Build PhysX
echo "Building PhysX (this may take a while)..."
cd compiler/linux-release
make -j$(nproc)

# Install PhysX libraries and headers
echo "Installing PhysX to ${INSTALL_DIR}..."
sudo mkdir -p "${INSTALL_DIR}/include/PhysX"
sudo mkdir -p "${INSTALL_DIR}/lib"

# Copy headers
sudo cp -r ../../include/* "${INSTALL_DIR}/include/PhysX/"

# Copy libraries
sudo cp -r ../linux-release/bin/* "${INSTALL_DIR}/lib/" || true
sudo cp -r ../linux-release/lib/* "${INSTALL_DIR}/lib/" || true

# Update library cache
sudo ldconfig

echo "PhysX ${PHYSX_VERSION} installed successfully!"
echo "Headers: ${INSTALL_DIR}/include/PhysX"
echo "Libraries: ${INSTALL_DIR}/lib"

# Cleanup
cd /
rm -rf "$BUILD_DIR"

echo "Installation complete."
