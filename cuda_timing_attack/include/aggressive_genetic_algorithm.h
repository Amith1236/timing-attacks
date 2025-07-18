#pragma once
#include <string>
#include <vector>
#include <random>
#include <memory>
#include <thread>
#include <atomic>
#include "../include/password_checker.h"
#include "../include/advanced_cuda_kernels.h"

struct AdvancedIndividual {
    std::string password;
    double fitness;
    int age;
    double diversity_score;
    
    AdvancedIndividual(const std::string& pwd = "", double fit = 0.0) 
        : password(pwd), fitness(fit), age(0), diversity_score(0.0) {}
};

class AggressiveGeneticAlgorithm {
private:
    PasswordChecker& checker;
    std::string charset;
    size_t population_size;
    size_t password_length;
    
    // Adaptive parameters
    double mutation_rate;
    double crossover_rate;
    double selection_pressure;
    int tournament_size;
    
    // Advanced features
    bool use_multi_gpu;
    bool use_streaming;
    bool use_adaptive_params;
    bool use_island_model;
    bool use_hybrid_strategy;
    
    // Island model parameters
    int num_islands;
    int migration_interval;
    double migration_rate;
    
    // Hybrid strategy parameters
    double local_search_probability;
    int local_search_iterations;
    
    // Performance tracking
    std::vector<double> fitness_history;
    std::vector<double> diversity_history;
    std::atomic<int> generations_without_improvement;
    
    // Random number generation
    std::random_device rd;
    std::mt19937 gen;
    
    // Population management
    std::vector<AdvancedIndividual> population;
    std::vector<std::vector<AdvancedIndividual>> islands;
    
    // CUDA memory pools
    char* d_population;
    double* d_fitness;
    int* d_results;
    char* d_parents;
    char* d_children;
    char* d_selected;
    
    // Multi-threading
    std::vector<std::thread> worker_threads;
    std::atomic<bool> stop_evolution;
    
public:
    AggressiveGeneticAlgorithm(PasswordChecker& pc, 
                               const std::string& chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ",
                               size_t pop_size = 2048,
                               bool multi_gpu = true,
                               bool streaming = true,
                               bool adaptive = true,
                               bool island = false,
                               bool hybrid = true);
    
    ~AggressiveGeneticAlgorithm();
    
    // Core genetic algorithm
    std::string run(int max_generations = 2000);
    void evolve();
    
    // Advanced evaluation strategies
    void evaluateFitnessAdvanced();
    void evaluateFitnessMultiGPU();
    void evaluateFitnessStreaming();
    
    // Selection strategies
    std::vector<AdvancedIndividual> tournamentSelection();
    std::vector<AdvancedIndividual> rankSelection();
    std::vector<AdvancedIndividual> diversitySelection();
    
    // Crossover strategies
    AdvancedIndividual uniformCrossover(const AdvancedIndividual& parent1, const AdvancedIndividual& parent2);
    AdvancedIndividual twoPointCrossover(const AdvancedIndividual& parent1, const AdvancedIndividual& parent2);
    AdvancedIndividual adaptiveCrossover(const AdvancedIndividual& parent1, const AdvancedIndividual& parent2);
    
    // Mutation strategies
    void uniformMutation(AdvancedIndividual& individual);
    void adaptiveMutation(AdvancedIndividual& individual);
    void targetedMutation(AdvancedIndividual& individual);
    
    // Island model
    void initializeIslands();
    void evolveIslands();
    void migrateBetweenIslands();
    void mergeIslands();
    
    // Hybrid strategies
    void localSearch(AdvancedIndividual& individual);
    void hillClimbing(AdvancedIndividual& individual);
    void simulatedAnnealing(AdvancedIndividual& individual);
    
    // Adaptive parameter adjustment
    void adjustParameters();
    void updateSelectionPressure();
    void updateMutationRate();
    void updateCrossoverRate();
    
    // Diversity management
    double calculateDiversity();
    void maintainDiversity();
    void injectRandomIndividuals();
    
    // Performance monitoring
    void trackPerformance();
    bool hasConverged();
    bool shouldRestart();
    
    // Memory management
    void allocateCUDAMemory();
    void freeCUDAMemory();
    void optimizeMemoryUsage();
    
    // Utility functions
    void initializePopulation();
    void generateNextGeneration(const std::vector<AdvancedIndividual>& selected);
    bool isPasswordFound() const;
    AdvancedIndividual getBestIndividual() const;
    void printAdvancedStats(int generation) const;
    
    // Configuration
    void setMultiGPU(bool enabled) { use_multi_gpu = enabled; }
    void setStreaming(bool enabled) { use_streaming = enabled; }
    void setAdaptive(bool enabled) { use_adaptive_params = enabled; }
    void setIslandModel(bool enabled) { use_island_model = enabled; }
    void setHybridStrategy(bool enabled) { use_hybrid_strategy = enabled; }
    
    // Performance getters
    double getAverageFitness() const;
    double getBestFitness() const;
    double getDiversity() const;
    int getGenerationsWithoutImprovement() const;
}; 