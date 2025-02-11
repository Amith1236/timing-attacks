import tkinter as tk
from tkinter import messagebox
import matplotlib.pyplot as plt
import numpy as np
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from GeneticModel import genetic_algorithm, generate_population
from Parameters import _password, valid_charset, pop_size, generations

class PasswordCrackerUI:
    def __init__(self, root):
        self.root = root
        self.root.title("Genetic Algorithm Password Cracker")

        # Default parameters
        self.population = generate_population(pop_size, len(_password), valid_charset)
        self.gen_count = 0
        self.max_gens = generations
        self.best_fitness = float("inf")  # Start with worst possible value
        self.avg_fitness = float("inf")
        self.cracked_password = None

        # UI Components
        self.create_controls()
        self.create_info_labels()
        self.create_canvas()

    def create_controls(self):
        """Creates buttons and parameter controls."""
        control_frame = tk.Frame(self.root)
        control_frame.pack(side=tk.TOP, fill=tk.X)

        self.run_one_btn = tk.Button(control_frame, text="Run 1 Generation", command=self.run_one_generation)
        self.run_ten_btn = tk.Button(control_frame, text="Run 10 Generations", command=self.run_ten_generations)
        self.run_till_solved_btn = tk.Button(control_frame, text="Run Till Solved", command=self.run_till_solved)
        self.reset_btn = tk.Button(control_frame, text="Reset", command=self.reset_algorithm)

        self.run_one_btn.pack(side=tk.LEFT, padx=5)
        self.run_ten_btn.pack(side=tk.LEFT, padx=5)
        self.run_till_solved_btn.pack(side=tk.LEFT, padx=5)
        self.reset_btn.pack(side=tk.LEFT, padx=5)

    def create_info_labels(self):
        """Creates labels to display best fitness and average fitness."""
        self.info_frame = tk.Frame(self.root)
        self.info_frame.pack(side=tk.TOP, fill=tk.X)

        self.best_fitness_label = tk.Label(self.info_frame, text="Best Fitness: N/A")
        self.avg_fitness_label = tk.Label(self.info_frame, text="Average Fitness: N/A")
        self.best_individual_label = tk.Label(self.info_frame, text="Best Individual: N/A")

        self.best_fitness_label.pack(side=tk.LEFT, padx=10)
        self.avg_fitness_label.pack(side=tk.LEFT, padx=10)
        self.best_individual_label.pack(side=tk.LEFT, padx=10)

    def create_canvas(self):
        """Creates matplotlib canvas for visualization."""
        self.fig, self.ax = plt.subplots()
        self.canvas = FigureCanvasTkAgg(self.fig, master=self.root)
        self.canvas.get_tk_widget().pack(side=tk.BOTTOM, fill=tk.BOTH, expand=True)

    def visualize(self, generation, population, fitness_scores, cracked_password):
        """Updates the heatmap and fitness labels after each generation."""
        self.ax.clear()
        self.ax.set_title(f"Generation {generation}")

        # Convert fitness scores into a heatmap
        heatmap_size = int(np.sqrt(len(fitness_scores)))
        reshaped_data = np.array(fitness_scores[:heatmap_size ** 2]).reshape((heatmap_size, heatmap_size))

        im = self.ax.imshow(reshaped_data, cmap="coolwarm", interpolation="nearest")  # Inverted colormap
        self.fig.colorbar(im, ax=self.ax)
        self.canvas.draw()

        # Update labels
        self.best_fitness = min(fitness_scores)
        self.avg_fitness = sum(fitness_scores) / len(fitness_scores)
        best_individual = population[fitness_scores.index(self.best_fitness)]

        self.best_fitness_label.config(text=f"Best Fitness: {self.best_fitness:.5f}")
        self.avg_fitness_label.config(text=f"Average Fitness: {self.avg_fitness:.5f}")
        self.best_individual_label.config(text=f"Best Individual: {best_individual}")

        # If password is cracked
        if cracked_password:
            self.cracked_password = cracked_password
            messagebox.showinfo("Success", f"Password Cracked: {cracked_password}")
            self.disable_buttons()

    def disable_buttons(self):
        """Disable buttons once the password is cracked."""
        self.run_one_btn.config(state=tk.DISABLED)
        self.run_ten_btn.config(state=tk.DISABLED)
        self.run_till_solved_btn.config(state=tk.DISABLED)

    def run_one_generation(self):
        """Runs a single generation of the genetic algorithm."""
        genetic_algorithm(self.population, 1, self.visualize)

    def run_ten_generations(self):
        """Runs ten generations."""
        genetic_algorithm(self.population, 10, self.visualize)

    def run_till_solved(self):
        """Runs until the password is cracked."""
        genetic_algorithm(self.population, self.max_gens, self.visualize)

    def reset_algorithm(self):
        """Resets the algorithm."""
        self.population = generate_population(pop_size, len(_password), valid_charset)
        self.gen_count = 0
        self.best_fitness = float("inf")
        self.avg_fitness = float("inf")
        self.cracked_password = None

        self.ax.clear()
        self.canvas.draw()

        # Enable buttons
        self.run_one_btn.config(state=tk.NORMAL)
        self.run_ten_btn.config(state=tk.NORMAL)
        self.run_till_solved_btn.config(state=tk.NORMAL)

        # Reset labels
        self.best_fitness_label.config(text="Best Fitness: N/A")
        self.avg_fitness_label.config(text="Average Fitness: N/A")
        self.best_individual_label.config(text="Best Individual: N/A")

if __name__ == "__main__":
    root = tk.Tk()
    app = PasswordCrackerUI(root)
    root.mainloop()
