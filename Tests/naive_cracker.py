import timeit
import numpy as np
from statistics import median
import random
import string
import matplotlib.pyplot as plt



#_password = "floccinaucinihilipilification"
#_password = "hippopotomonstrosesquippedaliophobia"
_password = "passwordpassword"

pass_length = len(_password)
valid_charset = string.ascii_lowercase
pop_size = 2000
#random.seed(10)
mutation_rate = 0.001
lambda_proportion = 0.8
generations = 10000
n_times = 1

def caesar_shift(text, shift):
    """Applies a simple Caesar cipher shift to a string."""
    return ''.join(chr((ord(c) - 32 + shift) % 95 + 32) for c in text)
c_pass = caesar_shift(_password, 10)

def naive_checker(attempt):
    n = len(_password)
    success = True
    if len(attempt) != n:
        success = False
    else:
        idx_array =[_ for _ in range(n)]
        random.shuffle(idx_array)
        for idx in idx_array:
            if attempt[idx] != _password[idx]:
                success = False
                break
        # for idx in range(n):
        #     if attempt[idx] != c_pass[idx]:
        #         break
    return success




#Validation
def validation_time(attempt, n = 1, f = naive_checker):
    times = []
    for _ in range(n):
        start_time = timeit.default_timer()
        cracked = f(attempt)
        end_time = timeit.default_timer()
        times.append(end_time - start_time)
    return median(times) if not cracked else -1



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

def crossover(parent1, parent2):
    child = []
    run_same = True

    for i in range(pass_length):
        if run_same and parent1[i] == parent2[i]:
            child.append(parent1[i])
        else:
            run_same = False
            child.append(random.choice([parent1[i], parent2[i]]))
    
    return "".join(child)

def crossover2(parent1, parent2):

    point = random.randint(0, pass_length  -1)
    child = parent1[:point] + parent2[point:]
    
    return child


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

def estimateP(currentGenerations, validPasswords):
    p_est = ((lambda_proportion)**(currentGenerations))*(validPasswords)
    return p_est


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

def next_generation(selected):
    next_generation = []
    
    while len(next_generation) < pop_size:

        parent1, parent2 = random.sample(selected, 2)

        child = crossover2(parent1, parent2)
        child = mutate(child)
        
        next_generation.append(child)
    
    return next_generation

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
    cracked_p = 0
    population = initial_population
    fitness_scores = fitness(population)
    print(population)
    print(fitness_scores)
    cracked = False


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
        if -1 in fitness_scores:
            cracked = True
            crackedPassword = population[fitness_scores.index(-1)]
            crackedGens = gen
            break
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
 
    if cracked:
        cracked_p = estimateP(crackedGens, fitness_scores.count(-1))
        print("Password Cracked:")
        print(crackedPassword)
        print("Attempts: " + str(pop_size*crackedGens*n_times))
        print("Estimated P: " + str(cracked_p))
    return population, fitness_scores, cracked_p, crackedGens


p_s = []
gen_s =[]
for i in range(5):
    
    final_population, final_fitness, cracked_p, cracked_g = genetic_algorithm(init_population, generations)
    p_s.append(cracked_p)
    gen_s.append(cracked_g)

print(gen_s)
print(p_s)

# Plot Probability of Cracking vs Generations
plt.figure(figsize=(8, 5))
plt.scatter(gen_s, p_s, label="Crack Probability", color="red", alpha=0.7)
plt.plot(gen_s, p_s, linestyle='--', color='blue', alpha=0.5)

plt.xlabel("Generations")
plt.ylabel("Crack Probability")
plt.title("Crack Probability vs. Generations")
plt.legend()
plt.grid(True)

plt.show()