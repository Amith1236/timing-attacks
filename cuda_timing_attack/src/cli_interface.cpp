#include "../include/cli_interface.h"
#include "../include/cuda_kernels.h"
#include "../include/advanced_cuda_kernels.h"
#include "../include/aggressive_genetic_algorithm.h"
#include <iostream>
#include <limits>
#include <algorithm>

CLIInterface::CLIInterface() 
    : checker("passwordPassword"),
      ga(checker, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ", 1024, 0.01, 0.5, 1000),
      default_password("passwordPassword"),
      charset("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"),
      population_size(1024),
      mutation_rate(0.01),
      selection_rate(0.5),
      max_generations(1000) {
}

void CLIInterface::run() {
    printBanner();
    
    // Check CUDA availability
    if (!isCudaAvailable()) {
        std::cout << "⚠️  Warning: CUDA not available. Using CPU-only mode." << std::endl;
    } else {
        std::cout << "✅ CUDA detected. GPU acceleration enabled." << std::endl;
        printCudaDeviceInfo();
    }
    
    std::cout << std::endl;
    
    while (true) {
        showMainMenu();
        std::string choice;
        std::getline(std::cin, choice);
        
        if (choice == "1") {
            setPassword();
        } else if (choice == "2") {
            configureParameters();
        } else if (choice == "3") {
            runAttack();
        } else if (choice == "4") {
            runAggressiveAttack();
        } else if (choice == "5") {
            demonstrateTimingLeak();
        } else if (choice == "6") {
            advancedSettings();
        } else if (choice == "7") {
            showHelp();
        } else if (choice == "8") {
            std::cout << "👋 Goodbye!" << std::endl;
            break;
        } else {
            std::cout << "❌ Invalid choice. Please try again." << std::endl;
        }
        
        std::cout << std::endl;
    }
}

void CLIInterface::showMainMenu() {
    std::cout << "╔══════════════════════════════════════════╗" << std::endl;
    std::cout << "║           CUDA Timing Attack             ║" << std::endl;
    std::cout << "╠══════════════════════════════════════════╣" << std::endl;
    std::cout << "║ 1. Set Target Password                   ║" << std::endl;
    std::cout << "║ 2. Configure Parameters                  ║" << std::endl;
    std::cout << "║ 3. Run Standard Attack                   ║" << std::endl;
    std::cout << "║ 4. Run Aggressive Attack                 ║" << std::endl;
    std::cout << "║ 5. Demonstrate Timing Leak               ║" << std::endl;
    std::cout << "║ 6. Advanced Settings                     ║" << std::endl;
    std::cout << "║ 7. Help                                  ║" << std::endl;
    std::cout << "║ 8. Exit                                  ║" << std::endl;
    std::cout << "╚══════════════════════════════════════════╝" << std::endl;
    std::cout << "Current password: " << default_password << std::endl;
    std::cout << "Enter your choice (1-8): ";
}

void CLIInterface::setPassword() {
    std::cout << "\n🔐 Set Target Password" << std::endl;
    std::cout << "Current password: " << default_password << std::endl;
    std::cout << "Valid characters: " << charset << std::endl;
    std::cout << "Enter new password (or press Enter to keep current): ";
    
    std::string new_password;
    std::getline(std::cin, new_password);
    
    if (!new_password.empty()) {
        if (validatePassword(new_password)) {
            default_password = new_password;
            checker = PasswordChecker(new_password);
            ga = GeneticAlgorithm(checker, charset, population_size, mutation_rate, selection_rate, max_generations);
            std::cout << "✅ Password set to: " << new_password << std::endl;
        } else {
            std::cout << "❌ Invalid password. Please use only valid characters." << std::endl;
        }
    } else {
        std::cout << "ℹ️  Keeping current password." << std::endl;
    }
}

void CLIInterface::configureParameters() {
    std::cout << "\n⚙️  Configure Parameters" << std::endl;
    printConfiguration();
    
    std::cout << "\nEnter new values (press Enter to keep current):" << std::endl;
    
    // Population size
    std::cout << "Population size [" << population_size << "]: ";
    std::string input;
    std::getline(std::cin, input);
    if (!input.empty()) {
        try {
            size_t new_size = std::stoul(input);
            if (new_size > 0 && new_size <= 10000) {
                population_size = new_size;
            } else {
                std::cout << "❌ Population size must be between 1 and 10000." << std::endl;
            }
        } catch (...) {
            std::cout << "❌ Invalid input." << std::endl;
        }
    }
    
    // Mutation rate
    std::cout << "Mutation rate [" << mutation_rate << "]: ";
    std::getline(std::cin, input);
    if (!input.empty()) {
        try {
            double new_rate = std::stod(input);
            if (new_rate >= 0.0 && new_rate <= 1.0) {
                mutation_rate = new_rate;
            } else {
                std::cout << "❌ Mutation rate must be between 0.0 and 1.0." << std::endl;
            }
        } catch (...) {
            std::cout << "❌ Invalid input." << std::endl;
        }
    }
    
    // Selection rate
    std::cout << "Selection rate [" << selection_rate << "]: ";
    std::getline(std::cin, input);
    if (!input.empty()) {
        try {
            double new_rate = std::stod(input);
            if (new_rate > 0.0 && new_rate <= 1.0) {
                selection_rate = new_rate;
            } else {
                std::cout << "❌ Selection rate must be between 0.0 and 1.0." << std::endl;
            }
        } catch (...) {
            std::cout << "❌ Invalid input." << std::endl;
        }
    }
    
    // Max generations
    std::cout << "Max generations [" << max_generations << "]: ";
    std::getline(std::cin, input);
    if (!input.empty()) {
        try {
            int new_gen = std::stoi(input);
            if (new_gen > 0 && new_gen <= 10000) {
                max_generations = new_gen;
            } else {
                std::cout << "❌ Max generations must be between 1 and 10000." << std::endl;
            }
        } catch (...) {
            std::cout << "❌ Invalid input." << std::endl;
        }
    }
    
    // Update genetic algorithm with new parameters
    ga = GeneticAlgorithm(checker, charset, population_size, mutation_rate, selection_rate, max_generations);
    
    std::cout << "✅ Parameters updated!" << std::endl;
}

void CLIInterface::runAttack() {
    std::cout << "\n🚀 Running CUDA Timing Attack" << std::endl;
    printConfiguration();
    
    std::cout << "\nPress Enter to start the attack...";
    std::cin.get();
    
    std::cout << "\nGeneration | Best Fitness    | Avg Fitness      | Best Individual" << std::endl;
    std::cout << "----------|------------------|------------------|----------------" << std::endl;
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    std::string result = ga.run();
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    std::cout << "\n⏱️  Attack completed in " << duration.count() << " ms" << std::endl;
    
    if (!result.empty()) {
        std::cout << "🎯 Cracked password: " << result << std::endl;
    }
}

void CLIInterface::runAggressiveAttack() {
    std::cout << "\n🚀 Running AGGRESSIVE CUDA Timing Attack" << std::endl;
    printConfiguration();
    
    // Create aggressive genetic algorithm
    AggressiveGeneticAlgorithm aggressive_ga(checker, charset, population_size * 2, 
                                            true, true, true, false, true);
    
    std::cout << "\n🔥 Aggressive Features Enabled:" << std::endl;
    std::cout << "   • Multi-GPU Support: " << (getAvailableGPUs() > 1 ? "✅" : "❌") << std::endl;
    std::cout << "   • CUDA Streaming: ✅" << std::endl;
    std::cout << "   • Adaptive Parameters: ✅" << std::endl;
    std::cout << "   • Hybrid Strategies: ✅" << std::endl;
    std::cout << "   • Advanced Timing: ✅" << std::endl;
    std::cout << "   • Diversity Management: ✅" << std::endl;
    
    std::cout << "\nPress Enter to start the aggressive attack...";
    std::cin.get();
    
    std::cout << "\nGen | Best Fitness    | Avg Fitness      | Diversity | Mut Rate | Best Individual" << std::endl;
    std::cout << "----|------------------|------------------|-----------|----------|----------------" << std::endl;
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    std::string result = aggressive_ga.run(max_generations * 2);
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    std::cout << "\n⏱️  Aggressive attack completed in " << duration.count() << " ms" << std::endl;
    
    if (!result.empty()) {
        std::cout << "🎯 Cracked password: " << result << std::endl;
    }
}

void CLIInterface::advancedSettings() {
    std::cout << "\n⚙️  Advanced Settings" << std::endl;
    std::cout << "====================" << std::endl;
    
    std::cout << "\nCurrent GPU Configuration:" << std::endl;
    int num_gpus = getAvailableGPUs();
    std::cout << "   Available GPUs: " << num_gpus << std::endl;
    std::cout << "   Optimal Block Size: " << getOptimalBlockSize() << std::endl;
    
    if (num_gpus > 1) {
        std::cout << "\nMulti-GPU Performance:" << std::endl;
        std::cout << "   • Parallel processing across " << num_gpus << " GPUs" << std::endl;
        std::cout << "   • Estimated speedup: " << num_gpus << "x" << std::endl;
        std::cout << "   • Memory distribution: " << population_size / num_gpus << " individuals per GPU" << std::endl;
    }
    
    std::cout << "\nAdvanced Features:" << std::endl;
    std::cout << "   • CUDA Streaming: Reduces memory latency" << std::endl;
    std::cout << "   • Adaptive Parameters: Self-tuning mutation and selection" << std::endl;
    std::cout << "   • Diversity Management: Prevents premature convergence" << std::endl;
    std::cout << "   • Local Search: Hill climbing for promising individuals" << std::endl;
    std::cout << "   • Tournament Selection: Better selection pressure" << std::endl;
    
    std::cout << "\nPerformance Tips:" << std::endl;
    std::cout << "   • Larger populations work better with aggressive mode" << std::endl;
    std::cout << "   • Multi-GPU systems see significant speedup" << std::endl;
    std::cout << "   • Adaptive parameters reduce manual tuning" << std::endl;
    std::cout << "   • Diversity management prevents getting stuck" << std::endl;
}

void CLIInterface::demonstrateTimingLeak() {
    std::cout << "\n🔍 Demonstrating Timing Leak" << std::endl;
    std::cout << "Testing password: " << default_password << std::endl;
    
    std::vector<std::string> test_passwords = {
        "a",  // Wrong length
        "p",  // Wrong first char
        "pa", // Wrong second char
        "pas", // Wrong third char
        default_password.substr(0, 1), // Correct first char
        default_password.substr(0, 2), // Correct first two chars
        default_password.substr(0, 3), // Correct first three chars
        default_password // Correct password
    };
    
    std::cout << "\nPassword Attempt          | Time (ns) | Correct Chars" << std::endl;
    std::cout << "---------------------------|-----------|--------------" << std::endl;
    
    for (const auto& attempt : test_passwords) {
        double time = checker.measureCheckTime(attempt, 1000);
        int correct_chars = 0;
        
        for (size_t i = 0; i < std::min(attempt.length(), default_password.length()); ++i) {
            if (attempt[i] == default_password[i]) {
                correct_chars++;
            } else {
                break;
            }
        }
        
        std::cout << std::setw(25) << attempt << " | "
                  << std::setw(9) << std::fixed << std::setprecision(0) << time << " | "
                  << std::setw(12) << correct_chars << std::endl;
    }
    
    std::cout << "\n💡 Notice how longer times correspond to more correct characters!" << std::endl;
}

void CLIInterface::showHelp() {
    std::cout << "\n📖 Help - CUDA Timing Attack" << std::endl;
    std::cout << "=============================" << std::endl;
    std::cout << "\nThis program demonstrates a timing attack against a vulnerable password checker." << std::endl;
    std::cout << "\nHow it works:" << std::endl;
    std::cout << "1. The password checker has a timing leak - it exits early on first mismatch" << std::endl;
    std::cout << "2. This creates timing differences that reveal information about correct characters" << std::endl;
    std::cout << "3. A genetic algorithm uses these timing differences as fitness scores" << std::endl;
    std::cout << "4. CUDA accelerates the fitness evaluation by checking multiple passwords in parallel" << std::endl;
    std::cout << "\nParameters:" << std::endl;
    std::cout << "- Population size: Number of password attempts per generation" << std::endl;
    std::cout << "- Mutation rate: Probability of changing a character during mutation" << std::endl;
    std::cout << "- Selection rate: Proportion of best individuals selected for reproduction" << std::endl;
    std::cout << "- Max generations: Maximum number of generations to run" << std::endl;
    std::cout << "\nSecurity implications:" << std::endl;
    std::cout << "This demonstrates why constant-time password comparison is crucial for security." << std::endl;
}

void CLIInterface::printBanner() {
    std::cout << "╔══════════════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║                    CUDA Timing Attack                        ║" << std::endl;
    std::cout << "║              Genetic Algorithm Password Cracker              ║" << std::endl;
    std::cout << "║                    Educational Tool                          ║" << std::endl;
    std::cout << "╚══════════════════════════════════════════════════════════════╝" << std::endl;
}

void CLIInterface::printConfiguration() {
    std::cout << "Current Configuration:" << std::endl;
    std::cout << "  Target Password: " << default_password << std::endl;
    std::cout << "  Character Set: " << charset << std::endl;
    std::cout << "  Population Size: " << population_size << std::endl;
    std::cout << "  Mutation Rate: " << mutation_rate << std::endl;
    std::cout << "  Selection Rate: " << selection_rate << std::endl;
    std::cout << "  Max Generations: " << max_generations << std::endl;
}

bool CLIInterface::validatePassword(const std::string& password) {
    if (password.empty()) {
        return false;
    }
    
    return std::all_of(password.begin(), password.end(), 
                       [this](char c) { return charset.find(c) != std::string::npos; });
}

std::string CLIInterface::getValidPassword() {
    std::string password;
    do {
        std::cout << "Enter password (using characters from " << charset << "): ";
        std::getline(std::cin, password);
    } while (!validatePassword(password));
    
    return password;
} 