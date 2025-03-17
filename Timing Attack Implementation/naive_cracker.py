import timeit
from statistics import median
import random
import string
import matplotlib.pyplot as plt

# Set Password Phrase
# _password = "floccinaucinihilipilification"
# _password = "hippopotomonstrosesquippedaliophobia"
_password = "passwordpassword"

# Set Genetic Algorithm Parameters
valid_charset = (
    string.ascii_lowercase +
    string.ascii_uppercase
 )                                      # Allowed character set for password attempts
'''You may add string.ascii_uppercase, string.digits or any other valid charset'''
pop_size = 2000                         # Population size for the genetic algorithm
mutation_rate = 0.01                    # Probability of mutation per character
lambda_proportion = 0.8                 # Proportion of top individuals selected for reproduction
generations = 5000                      # Maximum number of generations
n_times = 1                             # Number of tries with the same password (for statistical precision)

# Auto set variables
pass_length = len(_password)  # Length of the target password


def naive_checker(attempt):
    """
    Naive Password Checking Algorithm:
    - Contains a timing vulnerability due to early exit on incorrect character
    """
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


def validation_time(attempt, n=1, f=naive_checker):
    """
    Measures the median time taken to validate or invalidate a password attempt.
    """
    # Takes in f: function to check password, we can try this with non naive function
    times = []
    for _ in range(n):
        start_time = timeit.default_timer()
        cracked = f(attempt)
        end_time = timeit.default_timer()
        times.append(end_time - start_time)
    return median(times) if not cracked else -1


# Initialize the first population with random character sequences
init_population = [''.join(random.choice(valid_charset) for _ in range(pass_length)) for _ in range(pop_size)]


### Genetic Algorithm

def fitness(population, f=naive_checker):
    """
    Evaluates the fitness of each individual by measuring validation time.
    """
    n_repeats = 1
    timings = []
    for individual in population:
        ind_time = validation_time(individual, n_repeats, f)
        timings.append(ind_time)
    return timings


def mutate(individual):
    """
    Mutates an individual by randmly changing characters with a small probability.
    """
    individual = list(individual)
    for i in range(len(individual)):
        if random.random() < mutation_rate:
            new_char = random.choice(valid_charset)
            while new_char == individual[i]:  # Ensure mutation produces a different character
                new_char = random.choice(valid_charset)
            individual[i] = new_char
    return ''.join(individual)


def top_prop_selection(population, fitness):
    """
    Selects the top proportion of the population based on fitness scores.
    """
    sorted_timings = sorted(enumerate(fitness), key=lambda x: x[1], reverse=True)
    sorted_population = [population[fitness_pair[0]] for fitness_pair in sorted_timings]
    selected_count = int(len(population) * lambda_proportion)
    selected = sorted_population[:selected_count]
    return selected



def next_generation_duplicate(selected):
    """
    Generate new poplation where children are direct mutations of parents.
    """
    next_generation = []
    idx = 0
    while len(next_generation) < pop_size:
        for _ in range(2):
            next_generation.append(mutate(selected[idx]))
        idx += 1
    return next_generation


###
### Alternative next gneration method using crossover from 2 parents
###

def next_generation(selected):
    """
    Generates a new population using crossover and mutation from selected individuals.
    """
    next_generation = []
    while len(next_generation) < pop_size:
        parent1, parent2 = random.sample(selected, 2)
        child = crossover(parent1, parent2)
        child = mutate(child)
        next_generation.append(child)
    return next_generation

def crossover(parent1, parent2):
    """
    Combines two parents to create a child by splitting at a random point.
    """
    point = random.randint(0, pass_length - 1)
    child = parent1[:point] + parent2[point:]
    return child


# Tracking Storage
best_fitness_storage = []
avg_fitness_storage = []


def genetic_algorithm(initial_population, generations):
    """
    Runs the genetic algorithm for password cracking.
    """
    population = initial_population
    fitness_scores = fitness(population)
    print(population)
    print(fitness_scores)
    cracked = False

    for gen in range(generations):
        print(f"Generation {gen + 1}")
        selected = top_prop_selection(population, fitness_scores)
        population = next_generation_duplicate(selected)
        fitness_scores = fitness(population)
        if -1 in fitness_scores:
            cracked = True
            crackedPassword = population[fitness_scores.index(-1)]
            crackedGens = gen
            break
        
        best_fitness = max(fitness_scores)
        best_individual = population[fitness_scores.index(best_fitness)]
        avg_fitness = sum(fitness_scores) / len(fitness_scores)
        
        print(f"Best Fitness: {best_fitness}")
        #print(f"Average Fitness: {avg_fitness}")
        print(f"Best Individual: {best_individual}")
        
        best_fitness_storage.append(best_fitness)
        avg_fitness_storage.append(avg_fitness)
    
    if cracked:
        print("Password Cracked:")
        print(crackedPassword)
        print("Attempts: " + str(pop_size * crackedGens * n_times))
    return population, fitness_scores, crackedGens

gen_s = []
# for i in range(1):
#     final_population, final_fitness, cracked_g = genetic_algorithm(init_population, generations)
#     gen_s.append(cracked_g)




while True:
    # Prompt user for a password
    print("\nValid character set:", valid_charset)
    use_default = input(f"Do you want to use the default password '{_password}'? (yes/no): ").strip().lower()
    
    if use_default == 'no':
        while True:
            _password = input("Enter your custom password (using valid characters only): ").strip()
            if all(c in valid_charset for c in _password):
                break
            print("Invalid password! Please use only the valid characters.")

    pass_length = len(_password)  # Update password length

    # Reset tracking storage for new run
    best_fitness_storage = []
    avg_fitness_storage = []
    gen_s = []

    # Generate initial population
    init_population = [
        ''.join(random.choice(valid_charset) for _ in range(pass_length)) for _ in range(pop_size)
    ]

    # Run Genetic Algorithm
    for i in range(1):
        final_population, final_fitness, cracked_g = genetic_algorithm(init_population, generations)
        gen_s.append(cracked_g)

    # Ask if the user wants to plot the fitness data
    plot_choice = input("Would you like to plot fitness data? (yes/no): ").strip().lower()
    
    if plot_choice == 'yes':
        # plt.plot(best_fitness_storage, label="Best Fitness")
        plt.plot(avg_fitness_storage, label="Average Fitness")
        plt.xlabel("Generations")
        plt.ylabel("Fitness Score")
        plt.title("Fitness Progress Over Generations")
        plt.legend()
        plt.show()

    # Ask if the user wants to run again
    run_again = input("Run again with a different password? (yes/no): ").strip().lower()
    if run_again != 'yes':
        break
