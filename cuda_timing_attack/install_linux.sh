#!/bin/bash

# Linux Installation Script for CUDA Timing Attack
# Supports Ubuntu, Debian, CentOS, RHEL, Fedora

set -e

echo "🐧 Linux Installation Script for CUDA Timing Attack"
echo "=================================================="

# Detect Linux distribution
if [ -f /etc/os-release ]; then
    . /etc/os-release
    OS=$NAME
    VER=$VERSION_ID
else
    echo "Cannot detect Linux distribution"
    exit 1
fi

echo "Detected: $OS $VER"

# Function to install packages based on distribution
install_packages() {
    case $OS in
        *"Ubuntu"*|*"Debian"*)
            echo "Installing packages for Ubuntu/Debian..."
            sudo apt update
            sudo apt install -y build-essential cmake git curl wget
            ;;
        *"CentOS"*|*"Red Hat"*|*"Rocky"*|*"AlmaLinux"*)
            echo "Installing packages for CentOS/RHEL..."
            sudo yum groupinstall -y "Development Tools"
            sudo yum install -y cmake git curl wget
            ;;
        *"Fedora"*)
            echo "Installing packages for Fedora..."
            sudo dnf groupinstall -y "Development Tools"
            sudo dnf install -y cmake git curl wget
            ;;
        *"Arch"*)
            echo "Installing packages for Arch Linux..."
            sudo pacman -Syu --noconfirm base-devel cmake git curl wget
            ;;
        *)
            echo "Unsupported distribution: $OS"
            echo "Please install manually: build-essential, cmake, git, curl, wget"
            ;;
    esac
}

# Function to install CUDA
install_cuda() {
    echo "Installing CUDA Toolkit..."
    
    case $OS in
        *"Ubuntu"*|*"Debian"*)
            # Add NVIDIA repository
            wget https://developer.download.nvidia.com/compute/cuda/repos/ubuntu2204/x86_64/cuda-keyring_1.0-1_all.deb
            sudo dpkg -i cuda-keyring_1.0-1_all.deb
            sudo apt-get update
            sudo apt-get install -y cuda-toolkit-12-0
            rm cuda-keyring_1.0-1_all.deb
            ;;
        *"CentOS"*|*"Red Hat"*|*"Rocky"*|*"AlmaLinux"*)
            # Install CUDA via package manager
            sudo yum install -y cuda-toolkit
            ;;
        *"Fedora"*)
            sudo dnf install -y cuda-toolkit
            ;;
        *)
            echo "Please install CUDA manually from: https://developer.nvidia.com/cuda-downloads"
            ;;
    esac
}

# Function to setup environment
setup_environment() {
    echo "Setting up environment..."
    
    # Add CUDA to PATH
    if ! grep -q "cuda/bin" ~/.bashrc; then
        echo 'export PATH=/usr/local/cuda/bin:$PATH' >> ~/.bashrc
        echo 'export LD_LIBRARY_PATH=/usr/local/cuda/lib64:$LD_LIBRARY_PATH' >> ~/.bashrc
    fi
    
    # Source bashrc
    source ~/.bashrc
}

# Function to verify installation
verify_installation() {
    echo "Verifying installation..."
    
    # Check CUDA
    if command -v nvcc &> /dev/null; then
        echo "✅ CUDA installed: $(nvcc --version | grep release)"
    else
        echo "❌ CUDA not found"
        return 1
    fi
    
    # Check CMake
    if command -v cmake &> /dev/null; then
        echo "✅ CMake installed: $(cmake --version | head -n1)"
    else
        echo "❌ CMake not found"
        return 1
    fi
    
    # Check GPU
    if command -v nvidia-smi &> /dev/null; then
        echo "✅ NVIDIA GPU detected:"
        nvidia-smi --query-gpu=name,memory.total --format=csv,noheader,nounits
    else
        echo "⚠️  nvidia-smi not found (GPU drivers may not be installed)"
    fi
}

# Main installation process
echo "Step 1: Installing system packages..."
install_packages

echo "Step 2: Installing CUDA Toolkit..."
install_cuda

echo "Step 3: Setting up environment..."
setup_environment

echo "Step 4: Verifying installation..."
verify_installation

echo ""
echo "🎉 Installation complete!"
echo ""
echo "Next steps:"
echo "1. Restart your terminal or run: source ~/.bashrc"
echo "2. Build the project: ./build_linux.sh"
echo "3. Run the program: cd build/bin && ./cuda_timing_attack"
echo ""
echo "If you encounter issues:"
echo "- Check CUDA installation: nvcc --version"
echo "- Check GPU drivers: nvidia-smi"
echo "- Install NVIDIA drivers if needed: sudo ubuntu-drivers autoinstall" 