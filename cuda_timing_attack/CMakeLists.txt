cmake_minimum_required(VERSION 3.18)
project(CudaTimingAttack CUDA CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find CUDA
find_package(CUDA REQUIRED)

# Set CUDA flags
set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS};-O3;-arch=sm_60)

# Include directories
include_directories(${CUDA_INCLUDE_DIRS})

# Source files
set(SOURCES
    src/main.cpp
    src/timing_attack.cpp
    src/password_checker.cpp
    src/genetic_algorithm.cpp
    src/aggressive_genetic_algorithm.cpp
    src/cli_interface.cpp
)

# CUDA source files
set(CUDA_SOURCES
    src/cuda_kernels.cu
    src/advanced_cuda_kernels.cu
)

# Create executable
cuda_add_executable(cuda_timing_attack ${SOURCES} ${CUDA_SOURCES})

# Link libraries
target_link_libraries(cuda_timing_attack ${CUDA_LIBRARIES})

# Set output directory
set_target_properties(cuda_timing_attack PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
) 