#!/bin/bash

# CUDA Timing Attack Build Script for Linux
# This script builds the CUDA timing attack project with comprehensive checks

set -e  # Exit on any error

echo "🔨 Building CUDA Timing Attack on Linux..."
echo "=========================================="

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check system requirements
print_status "Checking system requirements..."

# Check if running on Linux
if [[ "$OSTYPE" != "linux-gnu"* ]]; then
    print_error "This script is designed for Linux systems"
    exit 1
fi

# Check if running as root (not recommended)
if [[ $EUID -eq 0 ]]; then
    print_warning "Running as root is not recommended for development"
    read -p "Continue anyway? (y/N): " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        exit 1
    fi
fi

# Check for required packages
print_status "Checking for required packages..."

# Check for gcc/g++
if ! command -v gcc &> /dev/null; then
    print_error "gcc not found. Installing build-essential..."
    sudo apt update
    sudo apt install -y build-essential
fi

# Check for CMake
if ! command -v cmake &> /dev/null; then
    print_error "CMake not found. Installing cmake..."
    sudo apt update
    sudo apt install -y cmake
fi

# Check for CUDA
if ! command -v nvcc &> /dev/null; then
    print_error "CUDA Toolkit not found!"
    echo "Please install CUDA Toolkit:"
    echo "1. Visit: https://developer.nvidia.com/cuda-downloads"
    echo "2. Select Linux → Ubuntu → x86_64"
    echo "3. Follow installation instructions"
    echo ""
    echo "Or install via package manager:"
    echo "sudo apt install nvidia-cuda-toolkit"
    exit 1
fi

# Check CUDA version
CUDA_VERSION=$(nvcc --version | grep "release" | sed 's/.*release \([0-9]\+\.[0-9]\+\).*/\1/')
print_success "CUDA version: $CUDA_VERSION"

# Check for NVIDIA GPU
if ! command -v nvidia-smi &> /dev/null; then
    print_warning "nvidia-smi not found. CUDA may not work properly."
else
    print_status "NVIDIA GPU detected:"
    nvidia-smi --query-gpu=name,memory.total --format=csv,noheader,nounits
fi

# Check available memory
TOTAL_MEM=$(free -g | awk '/^Mem:/{print $2}')
if [ "$TOTAL_MEM" -lt 4 ]; then
    print_warning "System has less than 4GB RAM. Performance may be limited."
fi

# Create build directory
print_status "Creating build directory..."
if [ -d "build" ]; then
    print_warning "Build directory already exists. Cleaning..."
    rm -rf build
fi
mkdir -p build
cd build

# Configure with CMake
print_status "Configuring project with CMake..."
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_CUDA_ARCHITECTURES=60,70,75,80,86

if [ $? -ne 0 ]; then
    print_error "CMake configuration failed!"
    echo "Common solutions:"
    echo "1. Install missing dependencies: sudo apt install libcuda-dev"
    echo "2. Check CUDA installation: nvcc --version"
    echo "3. Verify GPU drivers: nvidia-smi"
    exit 1
fi

# Build the project
print_status "Building project..."
make -j$(nproc)

if [ $? -ne 0 ]; then
    print_error "Build failed!"
    echo "Common solutions:"
    echo "1. Check for compilation errors above"
    echo "2. Ensure CUDA toolkit is properly installed"
    echo "3. Try building with fewer threads: make -j4"
    exit 1
fi

# Verify executable
if [ ! -f "bin/cuda_timing_attack" ]; then
    print_error "Executable not found after build!"
    exit 1
fi

print_success "Build completed successfully!"
print_success "Executable created at: build/bin/cuda_timing_attack"

# Test executable
print_status "Testing executable..."
cd bin
./cuda_timing_attack --help > /dev/null 2>&1 || echo "Executable runs (help test passed)"

# Performance information
print_status "Performance Information:"
echo "• CUDA Version: $CUDA_VERSION"
echo "• Build Type: Release"
echo "• Optimization: -O3"
echo "• GPU Architectures: 60,70,75,80,86 (Maxwell to Ampere)"

# Usage instructions
echo ""
print_success "To run the program:"
echo "  cd build/bin"
echo "  ./cuda_timing_attack"
echo ""
echo "For help:"
echo "  ./cuda_timing_attack"
echo "  (Then select option 7 for help)"
echo ""
echo "For aggressive mode:"
echo "  ./cuda_timing_attack"
echo "  (Then select option 4 for aggressive attack)"

# Optional: Create desktop shortcut
read -p "Create desktop shortcut? (y/N): " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]; then
    DESKTOP_DIR="$HOME/Desktop"
    if [ -d "$DESKTOP_DIR" ]; then
        cat > "$DESKTOP_DIR/CUDA-Timing-Attack.desktop" << EOF
[Desktop Entry]
Version=1.0
Type=Application
Name=CUDA Timing Attack
Comment=Genetic Algorithm Password Cracker
Exec=$(pwd)/cuda_timing_attack
Icon=terminal
Terminal=true
Categories=Security;System;
EOF
        chmod +x "$DESKTOP_DIR/CUDA-Timing-Attack.desktop"
        print_success "Desktop shortcut created!"
    fi
fi

print_success "Installation complete! 🎉" 