#!/bin/bash

# CUDA Timing Attack Build Script
# This script builds the CUDA timing attack project

echo "🔨 Building CUDA Timing Attack..."

# Check if CUDA is available
if ! command -v nvcc &> /dev/null; then
    echo "❌ Error: nvcc (NVIDIA CUDA Compiler) not found!"
    echo "Please install CUDA Toolkit from: https://developer.nvidia.com/cuda-downloads"
    exit 1
fi

# Check if CMake is available
if ! command -v cmake &> /dev/null; then
    echo "❌ Error: cmake not found!"
    echo "Please install CMake from: https://cmake.org/download/"
    exit 1
fi

# Create build directory
mkdir -p build
cd build

# Configure with CMake
echo "📋 Configuring project..."
cmake .. -DCMAKE_BUILD_TYPE=Release

if [ $? -ne 0 ]; then
    echo "❌ CMake configuration failed!"
    exit 1
fi

# Build the project
echo "🔨 Building project..."
make -j$(nproc)

if [ $? -ne 0 ]; then
    echo "❌ Build failed!"
    exit 1
fi

echo "✅ Build successful!"
echo "🚀 Executable created at: build/bin/cuda_timing_attack"
echo ""
echo "To run the program:"
echo "  cd build/bin"
echo "  ./cuda_timing_attack" 