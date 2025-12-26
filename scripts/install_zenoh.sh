#!/bin/bash
set -e

ZENOH_VERSION="1.7.1"
INSTALL_DIR="/usr/local"

echo "Installing Zenoh ${ZENOH_VERSION}..."

# Install dependencies
sudo apt-get update
sudo apt-get install -y libclang-dev curl build-essential cmake

# Install Rust if not already installed
if ! command -v cargo &> /dev/null; then
  echo "Installing Rust toolchain..."
  curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y
  source "$HOME/.cargo/env"
fi

# Download and build
cd /tmp
curl -L -o zenoh-c.tar.gz \
  "https://github.com/eclipse-zenoh/zenoh-c/archive/refs/tags/${ZENOH_VERSION}.tar.gz"
tar xzf zenoh-c.tar.gz
cd "zenoh-c-${ZENOH_VERSION}"

mkdir -p build && cd build
cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX="${INSTALL_DIR}" \
  -DZENOHC_BUILD_EXAMPLES=OFF \
  -DZENOHC_BUILD_TESTS=OFF

make -j$(nproc)
sudo make install
sudo ldconfig

echo "Zenoh ${ZENOH_VERSION} installed successfully"
echo "Verify installation:"
echo "  ls ${INSTALL_DIR}/lib/libzenohc*"
echo "  ls ${INSTALL_DIR}/include/zenoh.h"
