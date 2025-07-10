#!/bin/bash

set -e  # Exit immediately if any command fails

echo "🔨 Building project..."
cmake --build cmake-build-debug

echo "🚀 Running magma-voxel..."
./cmake-build-debug/magma-voxel
