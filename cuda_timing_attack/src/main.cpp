#include "../include/cli_interface.h"
#include "../include/cuda_kernels.h"
#include <iostream>
#include <cuda_runtime.h>

int main(int argc, char* argv[]) {
    try {
        // Initialize CUDA
        cudaError_t cuda_status = cudaSetDevice(0);
        if (cuda_status != cudaSuccess) {
            std::cerr << "Warning: Could not set CUDA device. Error: " 
                      << cudaGetErrorString(cuda_status) << std::endl;
        }
        
        // Create and run CLI interface
        CLIInterface cli;
        cli.run();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown error occurred." << std::endl;
        return 1;
    }
    
    return 0;
} 