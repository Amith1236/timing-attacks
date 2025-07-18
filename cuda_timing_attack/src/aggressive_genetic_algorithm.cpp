#include "../include/aggressive_genetic_algorithm.h"
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <cuda_runtime.h>
#include <chrono>

AggressiveGeneticAlgorithm::AggressiveGeneticAlgorithm(PasswordChecker& pc, 
                                                       const std::string& chars,
                                                       size_t pop_size,
                                                       bool multi_gpu,
                                                       bool streaming,
                                                       bool adaptive,
                                                       bool island,
                                                       bool hybrid)
    : checker(pc), charset(chars), population_size(pop_size), 
      password_length(pc.getPasswordLength()),
      mutation_rate(0.02), crossover_rate(0.8), selection_pressure(0.7), tournament_size(3),
      use_multi_gpu(multi_gpu), use_streaming(streaming), use_adaptive_params(adaptive),
      use_island_model(island), use_hybrid_strategy(hybrid),
      num_islands(4), migration_interval(10), migration_rate(0.1),
      local_search_probability(0.1), local_search_iterations(100),
      generations_without_improvement(0), gen(rd()), stop_evolution(false) {
    
    // Initialize CUDA memory
    allocateCUDAMemory();
    
    // Initialize population
    population.resize(population_size);
    
    // Initialize islands if using island model
    if (use_island_model) {
        initializeIslands();
    }
}

AggressiveGeneticAlgorithm::~AggressiveGeneticAlgorithm() {
    freeCUDAMemory();
}

std::string AggressiveGeneticAlgorithm::run(int max_generations) {
    initializePopulation();
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    for (int generation = 0; generation < max_generations && !stop_evolution; ++generation) {
        // Evolve population
        evolve();
        
        // Check if password is found
        if (isPasswordFound()) {
            auto end_time = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
            
            std::cout << "\n🎉 Password cracked in generation " << generation + 1 
                      << " (" << duration.count() << " ms)!" << std::endl;
            return getBestIndividual().password;
        }
        
        // Track performance
        trackPerformance();
        
        // Print statistics
        printAdvancedStats(generation);
        
        // Adaptive parameter adjustment
        if (use_adaptive_params) {
            adjustParameters();
        }
        
        // Island model operations
        if (use_island_model && generation % migration_interval == 0) {
            migrateBetweenIslands();
        }
        
        // Check for convergence
        if (hasConverged()) {
            std::cout << "\n⚠️  Population converged. Restarting..." << std::endl;
            initializePopulation();
            generations_without_improvement = 0;
        }
    }
    
    std::cout << "\n❌ Failed to crack password in " << max_generations << " generations." << std::endl;
    return "";
}

void AggressiveGeneticAlgorithm::evolve() {
    // Evaluate fitness using advanced strategies
    if (use_multi_gpu && getAvailableGPUs() > 1) {
        evaluateFitnessMultiGPU();
    } else if (use_streaming) {
        evaluateFitnessStreaming();
    } else {
        evaluateFitnessAdvanced();
    }
    
    // Selection
    std::vector<AdvancedIndividual> selected;
    if (use_adaptive_params) {
        selected = diversitySelection();
    } else {
        selected = tournamentSelection();
    }
    
    // Generate next generation
    generateNextGeneration(selected);
    
    // Apply hybrid strategies
    if (use_hybrid_strategy) {
        for (auto& individual : population) {
            if (individual.fitness != -1.0) {  // Not found password
                std::uniform_real_distribution<> dist(0.0, 1.0);
                if (dist(gen) < local_search_probability) {
                    localSearch(individual);
                }
            }
        }
    }
    
    // Maintain diversity
    maintainDiversity();
}

void AggressiveGeneticAlgorithm::evaluateFitnessAdvanced() {
    // Copy population to GPU
    for (size_t i = 0; i < population_size; ++i) {
        cudaMemcpy(d_population + i * password_length, 
                   population[i].password.c_str(), 
                   password_length * sizeof(char), 
                   cudaMemcpyHostToDevice);
    }
    
    // Launch advanced timing kernel with multiple iterations
    advancedTimingKernel(d_population, d_fitness, d_results, 
                        population_size, password_length, 25);  // 25 iterations
    
    // Copy results back
    std::vector<double> fitness_values(population_size);
    std::vector<int> results(population_size);
    
    cudaMemcpy(fitness_values.data(), d_fitness, 
               population_size * sizeof(double), cudaMemcpyDeviceToHost);
    cudaMemcpy(results.data(), d_results, 
               population_size * sizeof(int), cudaMemcpyDeviceToHost);
    
    // Update population
    for (size_t i = 0; i < population_size; ++i) {
        population[i].fitness = fitness_values[i];
        population[i].age++;
        if (results[i] == 1) {
            population[i].fitness = -1.0;  // Mark as found
        }
    }
}

void AggressiveGeneticAlgorithm::evaluateFitnessMultiGPU() {
    int num_gpus = getAvailableGPUs();
    if (num_gpus > 1) {
        std::vector<double> fitness_values(population_size);
        std::vector<int> results(population_size);
        
        // Copy population to host array
        std::vector<char> population_array(population_size * password_length);
        for (size_t i = 0; i < population_size; ++i) {
            std::copy(population[i].password.begin(), population[i].password.end(),
                      population_array.begin() + i * password_length);
        }
        
        // Multi-GPU evaluation
        evaluateFitnessMultiGPU(population_array.data(), fitness_values.data(), 
                               results.data(), population_size, password_length, num_gpus);
        
        // Update population
        for (size_t i = 0; i < population_size; ++i) {
            population[i].fitness = fitness_values[i];
            population[i].age++;
            if (results[i] == 1) {
                population[i].fitness = -1.0;
            }
        }
    } else {
        evaluateFitnessAdvanced();
    }
}

void AggressiveGeneticAlgorithm::evaluateFitnessStreaming() {
    streamingFitnessEvaluation(d_population, d_fitness, d_results,
                              population_size, password_length, 512);
    
    // Copy results back
    std::vector<double> fitness_values(population_size);
    std::vector<int> results(population_size);
    
    cudaMemcpy(fitness_values.data(), d_fitness, 
               population_size * sizeof(double), cudaMemcpyDeviceToHost);
    cudaMemcpy(results.data(), d_results, 
               population_size * sizeof(int), cudaMemcpyDeviceToHost);
    
    // Update population
    for (size_t i = 0; i < population_size; ++i) {
        population[i].fitness = fitness_values[i];
        population[i].age++;
        if (results[i] == 1) {
            population[i].fitness = -1.0;
        }
    }
}

std::vector<AdvancedIndividual> AggressiveGeneticAlgorithm::tournamentSelection() {
    std::vector<AdvancedIndividual> selected;
    selected.reserve(population_size / 2);
    
    std::uniform_int_distribution<> dist(0, population_size - 1);
    
    for (size_t i = 0; i < population_size / 2; ++i) {
        // Tournament selection
        int best_idx = dist(gen);
        double best_fitness = population[best_idx].fitness;
        
        for (int t = 1; t < tournament_size; ++t) {
            int candidate_idx = dist(gen);
            if (population[candidate_idx].fitness > best_fitness) {
                best_idx = candidate_idx;
                best_fitness = population[candidate_idx].fitness;
            }
        }
        
        selected.push_back(population[best_idx]);
    }
    
    return selected;
}

std::vector<AdvancedIndividual> AggressiveGeneticAlgorithm::diversitySelection() {
    std::vector<AdvancedIndividual> selected;
    selected.reserve(population_size / 2);
    
    // Sort by fitness and diversity
    std::vector<size_t> indices(population_size);
    std::iota(indices.begin(), indices.end(), 0);
    
    std::sort(indices.begin(), indices.end(), [this](size_t a, size_t b) {
        double score_a = population[a].fitness * (1.0 + population[a].diversity_score);
        double score_b = population[b].fitness * (1.0 + population[b].diversity_score);
        return score_a > score_b;
    });
    
    // Select top individuals
    for (size_t i = 0; i < population_size / 2; ++i) {
        selected.push_back(population[indices[i]]);
    }
    
    return selected;
}

AdvancedIndividual AggressiveGeneticAlgorithm::uniformCrossover(const AdvancedIndividual& parent1, 
                                                                const AdvancedIndividual& parent2) {
    AdvancedIndividual child;
    child.password.resize(password_length);
    
    std::uniform_real_distribution<> dist(0.0, 1.0);
    
    for (size_t i = 0; i < password_length; ++i) {
        if (dist(gen) < 0.5) {
            child.password[i] = parent1.password[i];
        } else {
            child.password[i] = parent2.password[i];
        }
    }
    
    return child;
}

void AggressiveGeneticAlgorithm::adaptiveMutation(AdvancedIndividual& individual) {
    std::uniform_real_distribution<> dist(0.0, 1.0);
    std::uniform_int_distribution<> char_dist(0, charset.length() - 1);
    
    // Adaptive mutation rate based on fitness and age
    double adaptive_rate = mutation_rate * (1.0 + individual.age * 0.1);
    if (individual.fitness < getAverageFitness()) {
        adaptive_rate *= 2.0;  // Higher mutation for poor performers
    }
    
    for (size_t i = 0; i < password_length; ++i) {
        if (dist(gen) < adaptive_rate) {
            individual.password[i] = charset[char_dist(gen)];
        }
    }
}

void AggressiveGeneticAlgorithm::localSearch(AdvancedIndividual& individual) {
    std::string best_password = individual.password;
    double best_fitness = individual.fitness;
    
    std::uniform_int_distribution<> pos_dist(0, password_length - 1);
    std::uniform_int_distribution<> char_dist(0, charset.length() - 1);
    
    for (int iter = 0; iter < local_search_iterations; ++iter) {
        std::string candidate = individual.password;
        
        // Randomly change one character
        int pos = pos_dist(gen);
        char old_char = candidate[pos];
        candidate[pos] = charset[char_dist(gen)];
        
        // Evaluate candidate
        double candidate_fitness = checker.measureCheckTime(candidate, 10);
        
        if (candidate_fitness > best_fitness) {
            best_password = candidate;
            best_fitness = candidate_fitness;
        }
    }
    
    individual.password = best_password;
    individual.fitness = best_fitness;
}

void AggressiveGeneticAlgorithm::adjustParameters() {
    updateSelectionPressure();
    updateMutationRate();
    updateCrossoverRate();
}

void AggressiveGeneticAlgorithm::updateMutationRate() {
    double diversity = calculateDiversity();
    
    if (diversity < 0.1) {
        mutation_rate = std::min(0.1, mutation_rate * 1.5);  // Increase mutation
    } else if (diversity > 0.8) {
        mutation_rate = std::max(0.001, mutation_rate * 0.8);  // Decrease mutation
    }
}

double AggressiveGeneticAlgorithm::calculateDiversity() {
    if (population.empty()) return 0.0;
    
    double total_diversity = 0.0;
    int comparisons = 0;
    
    for (size_t i = 0; i < population.size(); ++i) {
        for (size_t j = i + 1; j < population.size(); ++j) {
            int differences = 0;
            for (size_t k = 0; k < password_length; ++k) {
                if (population[i].password[k] != population[j].password[k]) {
                    differences++;
                }
            }
            total_diversity += (double)differences / password_length;
            comparisons++;
        }
    }
    
    return comparisons > 0 ? total_diversity / comparisons : 0.0;
}

void AggressiveGeneticAlgorithm::maintainDiversity() {
    double diversity = calculateDiversity();
    
    if (diversity < 0.05) {
        injectRandomIndividuals();
    }
    
    // Update diversity scores
    for (auto& individual : population) {
        individual.diversity_score = diversity;
    }
}

void AggressiveGeneticAlgorithm::injectRandomIndividuals() {
    std::uniform_int_distribution<> char_dist(0, charset.length() - 1);
    
    // Replace 10% of population with random individuals
    int replace_count = population_size / 10;
    
    for (int i = 0; i < replace_count; ++i) {
        std::string random_password;
        for (size_t j = 0; j < password_length; ++j) {
            random_password += charset[char_dist(gen)];
        }
        
        // Replace worst individual
        auto worst = std::min_element(population.begin(), population.end(),
                                     [](const AdvancedIndividual& a, const AdvancedIndividual& b) {
                                         return a.fitness < b.fitness;
                                     });
        worst->password = random_password;
        worst->fitness = 0.0;
        worst->age = 0;
    }
}

void AggressiveGeneticAlgorithm::trackPerformance() {
    double best_fitness = getBestFitness();
    double avg_fitness = getAverageFitness();
    double diversity = calculateDiversity();
    
    fitness_history.push_back(best_fitness);
    diversity_history.push_back(diversity);
    
    // Check for improvement
    if (fitness_history.size() > 1) {
        if (best_fitness <= fitness_history[fitness_history.size() - 2]) {
            generations_without_improvement++;
        } else {
            generations_without_improvement = 0;
        }
    }
}

bool AggressiveGeneticAlgorithm::hasConverged() {
    return generations_without_improvement > 50 || calculateDiversity() < 0.01;
}

void AggressiveGeneticAlgorithm::allocateCUDAMemory() {
    cudaMalloc(&d_population, population_size * password_length * sizeof(char));
    cudaMalloc(&d_fitness, population_size * sizeof(double));
    cudaMalloc(&d_results, population_size * sizeof(int));
    cudaMalloc(&d_parents, population_size * password_length * sizeof(char));
    cudaMalloc(&d_children, population_size * password_length * sizeof(char));
    cudaMalloc(&d_selected, population_size * password_length * sizeof(char));
}

void AggressiveGeneticAlgorithm::freeCUDAMemory() {
    cudaFree(d_population);
    cudaFree(d_fitness);
    cudaFree(d_results);
    cudaFree(d_parents);
    cudaFree(d_children);
    cudaFree(d_selected);
}

void AggressiveGeneticAlgorithm::initializePopulation() {
    std::uniform_int_distribution<> char_dist(0, charset.length() - 1);
    
    for (size_t i = 0; i < population_size; ++i) {
        std::string password;
        for (size_t j = 0; j < password_length; ++j) {
            password += charset[char_dist(gen)];
        }
        population[i] = AdvancedIndividual(password, 0.0);
    }
}

void AggressiveGeneticAlgorithm::generateNextGeneration(const std::vector<AdvancedIndividual>& selected) {
    std::vector<AdvancedIndividual> new_population;
    new_population.reserve(population_size);
    
    // Elitism: keep best individual
    auto best = std::max_element(population.begin(), population.end(),
                                [](const AdvancedIndividual& a, const AdvancedIndividual& b) {
                                    return a.fitness < b.fitness;
                                });
    new_population.push_back(*best);
    
    // Generate rest through crossover and mutation
    while (new_population.size() < population_size) {
        std::uniform_int_distribution<> parent_dist(0, selected.size() - 1);
        
        size_t parent1_idx = parent_dist(gen);
        size_t parent2_idx = parent_dist(gen);
        
        AdvancedIndividual child = uniformCrossover(selected[parent1_idx], selected[parent2_idx]);
        adaptiveMutation(child);
        
        new_population.push_back(child);
    }
    
    population = std::move(new_population);
}

bool AggressiveGeneticAlgorithm::isPasswordFound() const {
    for (const auto& individual : population) {
        if (individual.fitness == -1.0) {
            return true;
        }
    }
    return false;
}

AdvancedIndividual AggressiveGeneticAlgorithm::getBestIndividual() const {
    auto best = std::max_element(population.begin(), population.end(),
                                [](const AdvancedIndividual& a, const AdvancedIndividual& b) {
                                    return a.fitness < b.fitness;
                                });
    return *best;
}

void AggressiveGeneticAlgorithm::printAdvancedStats(int generation) const {
    auto best = getBestIndividual();
    double avg_fitness = getAverageFitness();
    double diversity = calculateDiversity();
    
    std::cout << std::setw(4) << generation + 1 << " | "
              << std::setw(15) << std::fixed << std::setprecision(2) << best.fitness << " | "
              << std::setw(15) << std::fixed << std::setprecision(2) << avg_fitness << " | "
              << std::setw(8) << std::fixed << std::setprecision(3) << diversity << " | "
              << std::setw(8) << mutation_rate << " | "
              << best.password << std::endl;
}

double AggressiveGeneticAlgorithm::getAverageFitness() const {
    double sum = 0.0;
    int count = 0;
    
    for (const auto& individual : population) {
        if (individual.fitness != -1.0) {
            sum += individual.fitness;
            count++;
        }
    }
    
    return count > 0 ? sum / count : 0.0;
}

double AggressiveGeneticAlgorithm::getBestFitness() const {
    return getBestIndividual().fitness;
}

double AggressiveGeneticAlgorithm::getDiversity() const {
    return calculateDiversity();
}

int AggressiveGeneticAlgorithm::getGenerationsWithoutImprovement() const {
    return generations_without_improvement;
}

// Stub implementations for unimplemented methods
void AggressiveGeneticAlgorithm::updateSelectionPressure() {
    // Implementation for adaptive selection pressure
}

void AggressiveGeneticAlgorithm::updateCrossoverRate() {
    // Implementation for adaptive crossover rate
}

void AggressiveGeneticAlgorithm::initializeIslands() {
    // Implementation for island model initialization
}

void AggressiveGeneticAlgorithm::evolveIslands() {
    // Implementation for island evolution
}

void AggressiveGeneticAlgorithm::migrateBetweenIslands() {
    // Implementation for island migration
}

void AggressiveGeneticAlgorithm::mergeIslands() {
    // Implementation for island merging
}

std::vector<AdvancedIndividual> AggressiveGeneticAlgorithm::rankSelection() {
    // Implementation for rank-based selection
    return tournamentSelection();  // Fallback
}

AdvancedIndividual AggressiveGeneticAlgorithm::twoPointCrossover(const AdvancedIndividual& parent1, 
                                                                 const AdvancedIndividual& parent2) {
    // Implementation for two-point crossover
    return uniformCrossover(parent1, parent2);  // Fallback
}

AdvancedIndividual AggressiveGeneticAlgorithm::adaptiveCrossover(const AdvancedIndividual& parent1, 
                                                                 const AdvancedIndividual& parent2) {
    // Implementation for adaptive crossover
    return uniformCrossover(parent1, parent2);  // Fallback
}

void AggressiveGeneticAlgorithm::uniformMutation(AdvancedIndividual& individual) {
    // Implementation for uniform mutation
    adaptiveMutation(individual);  // Fallback
}

void AggressiveGeneticAlgorithm::targetedMutation(AdvancedIndividual& individual) {
    // Implementation for targeted mutation
    adaptiveMutation(individual);  // Fallback
}

void AggressiveGeneticAlgorithm::hillClimbing(AdvancedIndividual& individual) {
    // Implementation for hill climbing
    localSearch(individual);  // Fallback
}

void AggressiveGeneticAlgorithm::simulatedAnnealing(AdvancedIndividual& individual) {
    // Implementation for simulated annealing
    localSearch(individual);  // Fallback
}

bool AggressiveGeneticAlgorithm::shouldRestart() {
    // Implementation for restart condition
    return hasConverged();
}

void AggressiveGeneticAlgorithm::optimizeMemoryUsage() {
    // Implementation for memory optimization
} 