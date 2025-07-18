#pragma once
#include <cuda_runtime.h>
#include <cuda.h>
#include <curand_kernel.h>

// Advanced CUDA kernel function declarations
extern "C" {
    // Multi-GPU fitness evaluation
    void evaluateFitnessMultiGPU(char* population, 
                                 double* fitness, 
                                 int* results,
                                 int population_size, 
                                 int password_length,
                                 int num_gpus);
    
    // Advanced timing measurement with multiple iterations
    void advancedTimingKernel(char* population,
                              double* fitness,
                              int* results,
                              int population_size,
                              int password_length,
                              int timing_iterations);
    
    // Parallel genetic operations on GPU
    void parallelCrossoverKernel(char* parents,
                                 char* children,
                                 int population_size,
                                 int password_length,
                                 unsigned long long seed);
    
    void parallelMutationKernel(char* population,
                                int population_size,
                                int password_length,
                                double mutation_rate,
                                unsigned long long seed);
    
    // Advanced selection with tournament selection
    void tournamentSelectionKernel(char* population,
                                   double* fitness,
                                   char* selected,
                                   int population_size,
                                   int password_length,
                                   int tournament_size,
                                   unsigned long long seed);
    
    // Memory-efficient streaming implementation
    void streamingFitnessEvaluation(char* population,
                                    double* fitness,
                                    int* results,
                                    int population_size,
                                    int password_length,
                                    int stream_size);
    
    // Adaptive population sizing
    void adaptivePopulationKernel(char* population,
                                  double* fitness,
                                  int* population_size,
                                  int password_length,
                                  double convergence_threshold);
    
    // Check if multiple GPUs are available
    int getAvailableGPUs();
    
    // Get optimal block size for current GPU
    int getOptimalBlockSize();
    
    // Memory pool management
    void* allocateMemoryPool(size_t size);
    void freeMemoryPool(void* ptr);
    
    // Advanced error checking
    void checkCudaError(const char* operation);
} 