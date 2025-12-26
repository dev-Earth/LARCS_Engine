#!/bin/bash
# Script to install Zenoh C library
# 
# For Ubuntu/Debian, Zenoh needs to be built from source or installed from binary releases
# This script provides guidance on installation

set -e

echo "=========================================="
echo "Zenoh C Library Installation (v1.7.1)"
echo "=========================================="
echo ""
echo "Option 1: Build from source (Recommended)"
echo "------------------------------------------"
echo "Prerequisites: Rust toolchain (https://rustup.rs)"
echo ""
echo "# Install Rust if not already installed"
echo "curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y"
echo "source \"\$HOME/.cargo/env\""
echo ""
echo "# Build and install Zenoh"
echo "cd /tmp"
echo "curl -L https://github.com/eclipse-zenoh/zenoh-c/archive/refs/tags/1.7.1.tar.gz | tar xz"
echo "cd zenoh-c-1.7.1"
echo "mkdir build && cd build"
echo "cmake -DCMAKE_BUILD_TYPE=Release .."
echo "make -j\$(nproc)  # This takes 5-10 minutes"
echo "sudo make install"
echo "sudo ldconfig"
echo ""
echo "Option 2: For CI/automated environments"
echo "----------------------------------------"
echo "See .github/workflows/ci.yml for reference"
echo ""
echo "=========================================="
echo "After installation, verify with:"
echo "  ls /usr/local/lib/libzenohc*"
echo "  ls /usr/local/include/zenoh.h"
echo "=========================================="
