import tkinter as tk
from tkinter import messagebox
import matplotlib.pyplot as plt
import numpy as np
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from GeneticModel import genetic_algorithm
from Parameters import _password, valid_charset, pop_size, generations
import string
import random

class PasswordCrackerUI:
    def __init__(self, root):
        self.root = root
        self.root.title("Genetic Algorithm Password Cracker")

        # Default parameters
        #self.population = generate_population(pop_size, len(_password), valid_charset)
        self.population = [''.join(random.choice(valid_charset) for _ in range(len(_password))) for _ in range (pop_size)]
        self.gen_count = 0
        self.max_gens = generations
        self.heatmap_data = None

        # UI Components
        self.create_controls()
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

    def create_canvas(self):
        """Creates matplotlib canvas for visualization."""
        self.fig, self.ax = plt.subplots()
        self.canvas = FigureCanvasTkAgg(self.fig, master=self.root)
        self.canvas.get_tk_widget().pack(side=tk.BOTTOM, fill=tk.BOTH, expand=True)

    def visualize(self, generation, population, fitness_scores, cracked_password):
        """Updates the heatmap after each generation."""
        self.ax.clear()
        self.ax.set_title(f"Generation {generation}")

        # Convert fitness scores into a heatmap
        heatmap_size = int(np.sqrt(len(fitness_scores)))
        reshaped_data = np.array(fitness_scores[:heatmap_size ** 2]).reshape((heatmap_size, heatmap_size))

        im = self.ax.imshow(reshaped_data, cmap="hot", interpolation="nearest")
        self.fig.colorbar(im, ax=self.ax)
        self.canvas.draw()

        if cracked_password:
            messagebox.showinfo("Success", f"Password Cracked: {cracked_password}")

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
        self.ax.clear()
        self.canvas.draw()

if __name__ == "__main__":
    root = tk.Tk()
    app = PasswordCrackerUI(root)
    root.mainloop()