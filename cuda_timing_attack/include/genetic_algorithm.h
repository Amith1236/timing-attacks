#pragma once
#include <string>
#include <vector>
#include <random>
#include "../include/password_checker.h"

struct Individual {
    std::string password;
    double fitness;
    
    Individual(const std::string& pwd = "", double fit = 0.0) 
        : password(pwd), fitness(fit) {}
};

class GeneticAlgorithm {
private:
    PasswordChecker& checker;
    std::string charset;
    size_t population_size;
    size_t password_length;
    double mutation_rate;
    double selection_rate;
    int max_generations;
    
    std::random_device rd;
    std::mt19937 gen;
    
    // Population
    std::vector<Individual> population;
    
    // CUDA buffers
    char* d_population;
    double* d_fitness;
    int* d_results;
    
public:
    GeneticAlgorithm(PasswordChecker& pc, 
                     const std::string& chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ",
                     size_t pop_size = 1024,
                     double mut_rate = 0.01,
                     double sel_rate = 0.5,
                     int max_gen = 1000);
    
    ~GeneticAlgorithm();
    
    // Initialize population
    void initializePopulation();
    
    // Run genetic algorithm
    std::string run();
    
    // Fitness evaluation (CUDA accelerated)
    void evaluateFitness();
    
    // Selection
    std::vector<Individual> select();
    
    // Crossover
    Individual crossover(const Individual& parent1, const Individual& parent2);
    
    // Mutation
    void mutate(Individual& individual);
    
    // Generate next generation
    void generateNextGeneration(const std::vector<Individual>& selected);
    
    // Check if password is found
    bool isPasswordFound() const;
    
    // Get best individual
    Individual getBestIndividual() const;
    
    // Print population statistics
    void printStats(int generation) const;
}; 