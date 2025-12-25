#!/bin/bash
# Script to download and install Zenoh C library from Rust installation

set -e

echo "Installing Zenoh C library from source..."

# Install Rust if not present
if ! command -v cargo &> /dev/null; then
    echo "Installing Rust..."
    curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y
    source "$HOME/.cargo/env"
fi

# Install cbindgen for generating C headers
cargo install cbindgen || true

TEMP_DIR=$(mktemp -d)
cd "$TEMP_DIR"

# Clone zenoh-c
echo "Cloning zenoh-c..."
git clone --depth 1 --branch 1.0.6 https://github.com/eclipse-zenoh/zenoh-c.git

cd zenoh-c

# Build
echo "Building zenoh-c..."
mkdir -p build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local ..
make -j$(nproc)

# Install
echo "Installing zenoh-c..."
sudo make install
sudo ldconfig

# Cleanup
cd /
rm -rf "$TEMP_DIR"

echo "Zenoh C library installed successfully!"
echo "Library location: /usr/local/lib"
echo "Headers location: /usr/local/include"

