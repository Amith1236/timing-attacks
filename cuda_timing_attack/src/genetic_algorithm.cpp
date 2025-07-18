#include "../include/genetic_algorithm.h"
#include "../include/cuda_kernels.h"
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <cuda_runtime.h>

GeneticAlgorithm::GeneticAlgorithm(PasswordChecker& pc, 
                                   const std::string& chars,
                                   size_t pop_size,
                                   double mut_rate,
                                   double sel_rate,
                                   int max_gen)
    : checker(pc), charset(chars), population_size(pop_size), 
      password_length(pc.getPasswordLength()), mutation_rate(mut_rate),
      selection_rate(sel_rate), max_generations(max_gen), gen(rd()) {
    
    // Allocate CUDA memory
    cudaMalloc(&d_population, population_size * password_length * sizeof(char));
    cudaMalloc(&d_fitness, population_size * sizeof(double));
    cudaMalloc(&d_results, population_size * sizeof(int));
    
    population.resize(population_size);
}

GeneticAlgorithm::~GeneticAlgorithm() {
    // Free CUDA memory
    cudaFree(d_population);
    cudaFree(d_fitness);
    cudaFree(d_results);
}

void GeneticAlgorithm::initializePopulation() {
    std::uniform_int_distribution<> char_dist(0, charset.length() - 1);
    
    for (size_t i = 0; i < population_size; ++i) {
        std::string password;
        for (size_t j = 0; j < password_length; ++j) {
            password += charset[char_dist(gen)];
        }
        population[i] = Individual(password, 0.0);
    }
}

std::string GeneticAlgorithm::run() {
    initializePopulation();
    
    for (int generation = 0; generation < max_generations; ++generation) {
        // Evaluate fitness using CUDA
        evaluateFitness();
        
        // Check if password is found
        if (isPasswordFound()) {
            std::cout << "\n🎉 Password cracked in generation " << generation + 1 << "!" << std::endl;
            return getBestIndividual().password;
        }
        
        // Print statistics
        printStats(generation);
        
        // Selection
        auto selected = select();
        
        // Generate next generation
        generateNextGeneration(selected);
    }
    
    std::cout << "\n❌ Failed to crack password in " << max_generations << " generations." << std::endl;
    return "";
}

void GeneticAlgorithm::evaluateFitness() {
    // Copy population to GPU
    for (size_t i = 0; i < population_size; ++i) {
        cudaMemcpy(d_population + i * password_length, 
                   population[i].password.c_str(), 
                   password_length * sizeof(char), 
                   cudaMemcpyHostToDevice);
    }
    
    // Launch CUDA kernel for fitness evaluation
    evaluateFitnessKernel(d_population, d_fitness, d_results, 
                         population_size, password_length);
    
    // Copy results back to CPU
    std::vector<double> fitness_values(population_size);
    std::vector<int> results(population_size);
    
    cudaMemcpy(fitness_values.data(), d_fitness, 
               population_size * sizeof(double), cudaMemcpyDeviceToHost);
    cudaMemcpy(results.data(), d_results, 
               population_size * sizeof(int), cudaMemcpyDeviceToHost);
    
    // Update population fitness
    for (size_t i = 0; i < population_size; ++i) {
        population[i].fitness = fitness_values[i];
        if (results[i] == 1) {  // Password found
            population[i].fitness = -1.0;  // Mark as found
        }
    }
}

std::vector<Individual> GeneticAlgorithm::select() {
    // Sort by fitness (higher is better for timing attacks)
    std::sort(population.begin(), population.end(), 
              [](const Individual& a, const Individual& b) {
                  return a.fitness > b.fitness;
              });
    
    size_t selected_count = static_cast<size_t>(population_size * selection_rate);
    std::vector<Individual> selected;
    selected.reserve(selected_count);
    
    for (size_t i = 0; i < selected_count; ++i) {
        selected.push_back(population[i]);
    }
    
    return selected;
}

Individual GeneticAlgorithm::crossover(const Individual& parent1, const Individual& parent2) {
    std::string child_password;
    std::uniform_real_distribution<> dist(0.0, 1.0);
    
    for (size_t i = 0; i < password_length; ++i) {
        if (dist(gen) < 0.5) {
            child_password += parent1.password[i];
        } else {
            child_password += parent2.password[i];
        }
    }
    
    return Individual(child_password, 0.0);
}

void GeneticAlgorithm::mutate(Individual& individual) {
    std::uniform_real_distribution<> dist(0.0, 1.0);
    std::uniform_int_distribution<> char_dist(0, charset.length() - 1);
    
    for (size_t i = 0; i < password_length; ++i) {
        if (dist(gen) < mutation_rate) {
            individual.password[i] = charset[char_dist(gen)];
        }
    }
}

void GeneticAlgorithm::generateNextGeneration(const std::vector<Individual>& selected) {
    std::vector<Individual> new_population;
    new_population.reserve(population_size);
    
    // Elitism: keep best individual
    new_population.push_back(selected[0]);
    
    // Generate rest of population through crossover and mutation
    while (new_population.size() < population_size) {
        std::uniform_int_distribution<> parent_dist(0, selected.size() - 1);
        
        size_t parent1_idx = parent_dist(gen);
        size_t parent2_idx = parent_dist(gen);
        
        Individual child = crossover(selected[parent1_idx], selected[parent2_idx]);
        mutate(child);
        
        new_population.push_back(child);
    }
    
    population = std::move(new_population);
}

bool GeneticAlgorithm::isPasswordFound() const {
    for (const auto& individual : population) {
        if (individual.fitness == -1.0) {
            return true;
        }
    }
    return false;
}

Individual GeneticAlgorithm::getBestIndividual() const {
    auto best = std::max_element(population.begin(), population.end(),
                                [](const Individual& a, const Individual& b) {
                                    return a.fitness < b.fitness;
                                });
    return *best;
}

void GeneticAlgorithm::printStats(int generation) const {
    auto best = getBestIndividual();
    double avg_fitness = 0.0;
    
    for (const auto& individual : population) {
        if (individual.fitness != -1.0) {
            avg_fitness += individual.fitness;
        }
    }
    avg_fitness /= population_size;
    
    std::cout << std::setw(4) << generation + 1 << " | "
              << std::setw(15) << std::fixed << std::setprecision(2) << best.fitness << " | "
              << std::setw(15) << std::fixed << std::setprecision(2) << avg_fitness << " | "
              << best.password << std::endl;
} 