#include "../include/cuda_kernels.h"
#include <cuda_runtime.h>
#include <device_launch_parameters.h>
#include <iostream>
#include <chrono>

// Target password (will be set by host)
__device__ char target_password[256];
__device__ int target_length = 0;

// CUDA kernel to check password with timing leak
__device__ bool checkPasswordKernel(const char* attempt, int length) {
    if (length != target_length) {
        return false;
    }
    
    // VULNERABLE: Early exit on first mismatch
    for (int i = 0; i < target_length; ++i) {
        if (attempt[i] != target_password[i]) {
            return false;  // Timing leak!
        }
    }
    
    return true;
}

// CUDA kernel to measure timing for password check
__device__ double measureCheckTime(const char* attempt, int length) {
    const int iterations = 10;  // Number of timing measurements
    double total_time = 0.0;
    
    for (int iter = 0; iter < iterations; ++iter) {
        // Simple timing simulation (in real implementation, this would use CUDA events)
        unsigned long long start_clock = clock64();
        
        checkPasswordKernel(attempt, length);
        
        unsigned long long end_clock = clock64();
        total_time += (double)(end_clock - start_clock);
    }
    
    return total_time / iterations;
}

// Main fitness evaluation kernel
__global__ void fitnessEvaluationKernel(char* population, 
                                        double* fitness, 
                                        int* results,
                                        int population_size, 
                                        int password_length) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    
    if (idx >= population_size) {
        return;
    }
    
    // Get individual's password
    const char* attempt = population + idx * password_length;
    
    // Check if password is correct
    bool is_correct = checkPasswordKernel(attempt, password_length);
    
    if (is_correct) {
        results[idx] = 1;  // Password found
        fitness[idx] = -1.0;  // Special value for found password
    } else {
        results[idx] = 0;
        // Measure timing for fitness (longer time = better fitness for timing attacks)
        fitness[idx] = measureCheckTime(attempt, password_length);
    }
}

// Host function to launch the kernel
extern "C" void evaluateFitnessKernel(char* population, 
                                      double* fitness, 
                                      int* results,
                                      int population_size, 
                                      int password_length) {
    // Set target password on device
    cudaMemcpyToSymbol(target_password, population, password_length);
    cudaMemcpyToSymbol(target_length, &password_length, sizeof(int));
    
    // Calculate grid and block dimensions
    int block_size = 256;
    int grid_size = (population_size + block_size - 1) / block_size;
    
    // Launch kernel
    fitnessEvaluationKernel<<<grid_size, block_size>>>(
        population, fitness, results, population_size, password_length);
    
    // Check for errors
    cudaError_t error = cudaGetLastError();
    if (error != cudaSuccess) {
        std::cerr << "CUDA Error: " << cudaGetErrorString(error) << std::endl;
    }
    
    // Synchronize
    cudaDeviceSynchronize();
}

// Check if CUDA is available
extern "C" bool isCudaAvailable() {
    int device_count;
    cudaError_t error = cudaGetDeviceCount(&device_count);
    
    if (error != cudaSuccess) {
        return false;
    }
    
    return device_count > 0;
}

// Print CUDA device information
extern "C" void printCudaDeviceInfo() {
    int device_count;
    cudaGetDeviceCount(&device_count);
    
    std::cout << "Found " << device_count << " CUDA device(s):" << std::endl;
    
    for (int i = 0; i < device_count; ++i) {
        cudaDeviceProp prop;
        cudaGetDeviceProperties(&prop, i);
        
        std::cout << "  Device " << i << ": " << prop.name << std::endl;
        std::cout << "    Compute Capability: " << prop.major << "." << prop.minor << std::endl;
        std::cout << "    Global Memory: " << prop.totalGlobalMem / (1024*1024*1024) << " GB" << std::endl;
        std::cout << "    Multiprocessors: " << prop.multiProcessorCount << std::endl;
        std::cout << "    Max Threads per Block: " << prop.maxThreadsPerBlock << std::endl;
        std::cout << "    Max Threads per SM: " << prop.maxThreadsPerMultiProcessor << std::endl;
    }
} 