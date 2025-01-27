import timeit
import numpy as np
from statistics import median
import random
import string



_password = "floccinaucinihilipilification"
#_password = "Hippopotomonstrosesquippedaliophobia"

pass_length = len(_password)
valid_charset = string.ascii_lowercase
pop_size = 2000
random.seed(1)
mutation_rate = 0.005
lambda_proportion = 0.8
generations = 10000


def naive_checker(attempt):
    n = len(_password)
    success = True
    if len(attempt) != n:
        success = False
    else:
        for idx in range(n):
            if attempt[idx] != _password[idx]:
                success = False
                break
    return success



#Validation
def validation_time(attempt, n = 1, f = naive_checker):
    times = []
    for _ in range(n):
        start_time = timeit.default_timer()
        f(attempt)
        end_time = timeit.default_timer()
        times.append(end_time - start_time)
    return median(times)



## Genetic Algorithm

# initial population


init_population = [''.join(random.choice(valid_charset) for _ in range(pass_length)) for _ in range (pop_size)]

#generate timings for each attempt
def fitness(population, f = naive_checker):
    n_repeats = 1
    timings = []
    for individual in population:
        ind_time = validation_time(individual, n_repeats, f)
        timings.append(ind_time)
    return timings

#constant mutate
def mutate(individual):
    individual = list(individual)
    
    for i in range(len(individual)):
        if random.random() < mutation_rate:  
            new_char = random.choice(valid_charset)
            # Ensure mutation produces a different character
            while new_char == individual[i]:  
                new_char = random.choice(valid_charset)
            individual[i] = new_char
    
    return ''.join(individual)


#Selection of best individuals
def top_prop_selection(population, fitness):
    sorted_timings = sorted(enumerate(fitness), key= lambda x: x[1], reverse=True)
    sorted_population = [population[fitness_pair[0]] for fitness_pair in sorted_timings]
    #print(sorted_timings)
    #rint(sorted_population)
    # Select lambda proportion of the population
    selected_count = int(len(population) * lambda_proportion)
    selected = []

    for i in range(selected_count):
        selected.append(sorted_population[i])
    
    return selected

def next_generation_duplicate(selected):
    next_generation = []
    idx = 0
    overflow = len(selected)

    while len(next_generation) < pop_size:
        child = mutate(selected[idx%overflow])
        next_generation.append(child)

    return next_generation

#tracking Storage
best_fitness_storage = []
avg_fitness_storage = []

def genetic_algorithm(initial_population, generations):
    population = initial_population
    fitness_scores = fitness(population)
    print(population)
    print(fitness_scores)



    for gen in range(generations):
        print(f"Generation {gen + 1}")
        
        # Roulette wheel selection
        #selected = roulette_wheel_selection(population, fitness_scores)
        selected = top_prop_selection(population, fitness_scores)
        #print(selected)
        
        # Generate next generation
        population = next_generation(selected)
        #population = next_generation_duplicate(selected)

        # Recalculate fitness for the new generation
        fitness_scores = fitness(population)
        
        # # Print best solution so far
        # best_fitness = max(fitness_scores)
        # best_individual = population[fitness_scores.index(best_fitness)]
        # print(f"Best Individual: {best_individual}, Fitness: {best_fitness}")
        # Calculate metrics
        best_fitness = max(fitness_scores)
        best_individual = population[fitness_scores.index(best_fitness)]
        avg_fitness = sum(fitness_scores) / len(fitness_scores)
        
        # Print metrics
        print(f"Best Fitness: {best_fitness}")
        print(f"Average Fitness: {avg_fitness}")
        print(f"Best Individual: {best_individual}")
        
        # Store metrics for plotting
        best_fitness_storage.append(best_fitness)
        avg_fitness_storage.append(avg_fitness)
 
    
    return population, fitness_scores



final_population, final_fitness = genetic_algorithm(init_population, generations)