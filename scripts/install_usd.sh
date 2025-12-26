#!/bin/bash
# USD (Universal Scene Description) Installation Script for LARCS Engine
# https://github.com/PixarAnimationStudios/USD

set -e

USD_VERSION="23.11"
USD_REPO="https://github.com/PixarAnimationStudios/USD.git"
INSTALL_DIR="/usr/local"
BUILD_DIR="/tmp/usd-build"

echo "Installing USD ${USD_VERSION}..."

# Install dependencies
echo "Installing build dependencies..."
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    git \
    python3 \
    python3-dev \
    python3-pip \
    nasm \
    libtbb-dev \
    libboost-all-dev \
    libglew-dev \
    libxrandr-dev \
    libxcursor-dev \
    libxinerama-dev \
    libxi-dev \
    zlib1g-dev

# Install Python dependencies
pip3 install --user PyOpenGL PySide6

# Clone USD repository
if [ -d "$BUILD_DIR" ]; then
    echo "Removing existing build directory..."
    rm -rf "$BUILD_DIR"
fi

echo "Cloning USD repository..."
git clone --depth 1 --branch "v${USD_VERSION}" "$USD_REPO" "$BUILD_DIR"

# Build and install USD
cd "$BUILD_DIR"

echo "Building USD (this may take a very long time - 30+ minutes)..."
python3 build_scripts/build_usd.py \
    --no-examples \
    --no-tutorials \
    --no-imaging \
    --no-usdview \
    "${INSTALL_DIR}/USD"

# Set up environment variables
echo "Setting up environment..."
cat >> ~/.bashrc << EOF

# USD Environment Variables
export USD_DIR=${INSTALL_DIR}/USD
export PATH=\${USD_DIR}/bin:\${PATH}
export LD_LIBRARY_PATH=\${USD_DIR}/lib:\${LD_LIBRARY_PATH}
export PYTHONPATH=\${USD_DIR}/lib/python:\${PYTHONPATH}
EOF

source ~/.bashrc || true

echo "USD ${USD_VERSION} installed successfully!"
echo "Installation directory: ${INSTALL_DIR}/USD"
echo "Please run: source ~/.bashrc"

# Cleanup
cd /
rm -rf "$BUILD_DIR"

echo "Installation complete."
