#include "../include/advanced_cuda_kernels.h"
#include <cuda_runtime.h>
#include <device_launch_parameters.h>
#include <curand_kernel.h>
#include <cuda_profiler_api.h>
#include <iostream>
#include <vector>
#include <algorithm>

// Global constants for optimization
__constant__ char target_password[256];
__constant__ int target_length;
__constant__ char valid_charset[128];
__constant__ int charset_size;

// Shared memory for timing measurements
__shared__ double shared_timings[256];

// Advanced password checking with multiple timing measurements
__device__ bool advancedCheckPassword(const char* attempt, int length) {
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

// Advanced timing measurement with statistical analysis
__device__ double advancedMeasureTiming(const char* attempt, int length, int iterations) {
    double total_time = 0.0;
    double min_time = 1e10;
    double max_time = 0.0;
    
    for (int iter = 0; iter < iterations; ++iter) {
        unsigned long long start_clock = clock64();
        
        advancedCheckPassword(attempt, length);
        
        unsigned long long end_clock = clock64();
        double time = (double)(end_clock - start_clock);
        
        total_time += time;
        min_time = min(min_time, time);
        max_time = max(max_time, time);
    }
    
    // Return weighted average (emphasize longer times for timing attacks)
    double avg_time = total_time / iterations;
    double variance = (max_time - min_time) / avg_time;
    
    // Boost fitness for consistent longer times
    return avg_time * (1.0 + variance);
}

// Advanced fitness evaluation kernel with multiple iterations
__global__ void advancedFitnessKernel(char* population, 
                                      double* fitness, 
                                      int* results,
                                      int population_size, 
                                      int password_length,
                                      int timing_iterations) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    
    if (idx >= population_size) {
        return;
    }
    
    // Get individual's password
    const char* attempt = population + idx * password_length;
    
    // Check if password is correct
    bool is_correct = advancedCheckPassword(attempt, password_length);
    
    if (is_correct) {
        results[idx] = 1;  // Password found
        fitness[idx] = -1.0;  // Special value for found password
    } else {
        results[idx] = 0;
        // Advanced timing measurement
        fitness[idx] = advancedMeasureTiming(attempt, password_length, timing_iterations);
    }
}

// Parallel crossover kernel
__global__ void crossoverKernel(char* parents,
                                char* children,
                                int population_size,
                                int password_length,
                                unsigned long long seed) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    
    if (idx >= population_size / 2) {
        return;
    }
    
    // Initialize random number generator
    curandState state;
    curand_init(seed, idx, 0, &state);
    
    // Get parent indices
    int parent1_idx = idx * 2;
    int parent2_idx = idx * 2 + 1;
    int child1_idx = idx * 2;
    int child2_idx = idx * 2 + 1;
    
    // Get parent passwords
    char* parent1 = parents + parent1_idx * password_length;
    char* parent2 = parents + parent2_idx * password_length;
    char* child1 = children + child1_idx * password_length;
    char* child2 = children + child2_idx * password_length;
    
    // Perform crossover
    for (int i = 0; i < password_length; ++i) {
        float rand_val = curand_uniform(&state);
        
        if (rand_val < 0.5) {
            child1[i] = parent1[i];
            child2[i] = parent2[i];
        } else {
            child1[i] = parent2[i];
            child2[i] = parent1[i];
        }
    }
}

// Parallel mutation kernel
__global__ void mutationKernel(char* population,
                               int population_size,
                               int password_length,
                               double mutation_rate,
                               unsigned long long seed) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    
    if (idx >= population_size) {
        return;
    }
    
    // Initialize random number generator
    curandState state;
    curand_init(seed, idx, 0, &state);
    
    char* password = population + idx * password_length;
    
    for (int i = 0; i < password_length; ++i) {
        float rand_val = curand_uniform(&state);
        
        if (rand_val < mutation_rate) {
            int char_idx = curand(&state) % charset_size;
            password[i] = valid_charset[char_idx];
        }
    }
}

// Tournament selection kernel
__global__ void tournamentSelectionKernel(char* population,
                                          double* fitness,
                                          char* selected,
                                          int population_size,
                                          int password_length,
                                          int tournament_size,
                                          unsigned long long seed) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    
    if (idx >= population_size) {
        return;
    }
    
    // Initialize random number generator
    curandState state;
    curand_init(seed, idx, 0, &state);
    
    // Tournament selection
    int best_idx = curand(&state) % population_size;
    double best_fitness = fitness[best_idx];
    
    for (int t = 1; t < tournament_size; ++t) {
        int candidate_idx = curand(&state) % population_size;
        if (fitness[candidate_idx] > best_fitness) {
            best_idx = candidate_idx;
            best_fitness = fitness[candidate_idx];
        }
    }
    
    // Copy selected individual
    char* source = population + best_idx * password_length;
    char* dest = selected + idx * password_length;
    
    for (int i = 0; i < password_length; ++i) {
        dest[i] = source[i];
    }
}

// Multi-GPU fitness evaluation
extern "C" void evaluateFitnessMultiGPU(char* population, 
                                        double* fitness, 
                                        int* results,
                                        int population_size, 
                                        int password_length,
                                        int num_gpus) {
    int gpus_per_batch = population_size / num_gpus;
    
    for (int gpu = 0; gpu < num_gpus; ++gpu) {
        cudaSetDevice(gpu);
        
        // Calculate batch size for this GPU
        int start_idx = gpu * gpus_per_batch;
        int end_idx = (gpu == num_gpus - 1) ? population_size : (gpu + 1) * gpus_per_batch;
        int batch_size = end_idx - start_idx;
        
        // Allocate device memory for this batch
        char* d_batch_population;
        double* d_batch_fitness;
        int* d_batch_results;
        
        cudaMalloc(&d_batch_population, batch_size * password_length * sizeof(char));
        cudaMalloc(&d_batch_fitness, batch_size * sizeof(double));
        cudaMalloc(&d_batch_results, batch_size * sizeof(int));
        
        // Copy batch to device
        cudaMemcpy(d_batch_population, 
                   population + start_idx * password_length,
                   batch_size * password_length * sizeof(char),
                   cudaMemcpyHostToDevice);
        
        // Launch kernel
        int block_size = getOptimalBlockSize();
        int grid_size = (batch_size + block_size - 1) / block_size;
        
        advancedFitnessKernel<<<grid_size, block_size>>>(
            d_batch_population, d_batch_fitness, d_batch_results,
            batch_size, password_length, 20);  // 20 timing iterations
        
        // Copy results back
        cudaMemcpy(fitness + start_idx, d_batch_fitness, 
                   batch_size * sizeof(double), cudaMemcpyDeviceToHost);
        cudaMemcpy(results + start_idx, d_batch_results, 
                   batch_size * sizeof(int), cudaMemcpyDeviceToHost);
        
        // Free device memory
        cudaFree(d_batch_population);
        cudaFree(d_batch_fitness);
        cudaFree(d_batch_results);
    }
}

// Advanced timing kernel with multiple iterations
extern "C" void advancedTimingKernel(char* population,
                                     double* fitness,
                                     int* results,
                                     int population_size,
                                     int password_length,
                                     int timing_iterations) {
    // Set target password on device
    cudaMemcpyToSymbol(target_password, population, password_length);
    cudaMemcpyToSymbol(target_length, &password_length, sizeof(int));
    
    // Calculate optimal grid and block dimensions
    int block_size = getOptimalBlockSize();
    int grid_size = (population_size + block_size - 1) / block_size;
    
    // Launch advanced kernel
    advancedFitnessKernel<<<grid_size, block_size>>>(
        population, fitness, results, population_size, password_length, timing_iterations);
    
    checkCudaError("Advanced timing kernel");
}

// Parallel crossover implementation
extern "C" void parallelCrossoverKernel(char* parents,
                                        char* children,
                                        int population_size,
                                        int password_length,
                                        unsigned long long seed) {
    int block_size = getOptimalBlockSize();
    int grid_size = (population_size / 2 + block_size - 1) / block_size;
    
    crossoverKernel<<<grid_size, block_size>>>(
        parents, children, population_size, password_length, seed);
    
    checkCudaError("Parallel crossover kernel");
}

// Parallel mutation implementation
extern "C" void parallelMutationKernel(char* population,
                                       int population_size,
                                       int password_length,
                                       double mutation_rate,
                                       unsigned long long seed) {
    int block_size = getOptimalBlockSize();
    int grid_size = (population_size + block_size - 1) / block_size;
    
    mutationKernel<<<grid_size, block_size>>>(
        population, population_size, password_length, mutation_rate, seed);
    
    checkCudaError("Parallel mutation kernel");
}

// Tournament selection implementation
extern "C" void tournamentSelectionKernel(char* population,
                                          double* fitness,
                                          char* selected,
                                          int population_size,
                                          int password_length,
                                          int tournament_size,
                                          unsigned long long seed) {
    int block_size = getOptimalBlockSize();
    int grid_size = (population_size + block_size - 1) / block_size;
    
    ::tournamentSelectionKernel<<<grid_size, block_size>>>(
        population, fitness, selected, population_size, password_length, tournament_size, seed);
    
    checkCudaError("Tournament selection kernel");
}

// Streaming fitness evaluation for large populations
extern "C" void streamingFitnessEvaluation(char* population,
                                           double* fitness,
                                           int* results,
                                           int population_size,
                                           int password_length,
                                           int stream_size) {
    cudaStream_t streams[3];
    for (int i = 0; i < 3; ++i) {
        cudaStreamCreate(&streams[i]);
    }
    
    int block_size = getOptimalBlockSize();
    int grid_size = (stream_size + block_size - 1) / block_size;
    
    for (int offset = 0; offset < population_size; offset += stream_size) {
        int current_stream = offset % 3;
        int current_size = min(stream_size, population_size - offset);
        
        // Launch kernel in stream
        advancedFitnessKernel<<<grid_size, block_size, 0, streams[current_stream]>>>(
            population + offset * password_length,
            fitness + offset,
            results + offset,
            current_size,
            password_length,
            15);
    }
    
    // Synchronize all streams
    for (int i = 0; i < 3; ++i) {
        cudaStreamSynchronize(streams[i]);
        cudaStreamDestroy(streams[i]);
    }
    
    checkCudaError("Streaming fitness evaluation");
}

// Utility functions
extern "C" int getAvailableGPUs() {
    int device_count;
    cudaGetDeviceCount(&device_count);
    return device_count;
}

extern "C" int getOptimalBlockSize() {
    cudaDeviceProp prop;
    cudaGetDeviceProperties(&prop, 0);
    
    // Optimal block size based on GPU capabilities
    int optimal_size = 256;
    
    if (prop.major >= 7) {  // Volta or newer
        optimal_size = 512;
    } else if (prop.major >= 6) {  // Pascal
        optimal_size = 384;
    }
    
    return min(optimal_size, prop.maxThreadsPerBlock);
}

extern "C" void* allocateMemoryPool(size_t size) {
    void* ptr;
    cudaMalloc(&ptr, size);
    return ptr;
}

extern "C" void freeMemoryPool(void* ptr) {
    cudaFree(ptr);
}

extern "C" void checkCudaError(const char* operation) {
    cudaError_t error = cudaGetLastError();
    if (error != cudaSuccess) {
        std::cerr << "CUDA Error in " << operation << ": " 
                  << cudaGetErrorString(error) << std::endl;
        throw std::runtime_error("CUDA operation failed");
    }
} 