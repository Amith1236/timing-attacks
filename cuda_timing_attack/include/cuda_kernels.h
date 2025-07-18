#pragma once
#include <cuda_runtime.h>

// CUDA kernel function declarations
extern "C" {
    // Evaluate fitness for all individuals in parallel
    void evaluateFitnessKernel(char* population, 
                               double* fitness, 
                               int* results,
                               int population_size, 
                               int password_length);
    
    // Check if CUDA is available
    bool isCudaAvailable();
    
    // Get CUDA device info
    void printCudaDeviceInfo();
} 