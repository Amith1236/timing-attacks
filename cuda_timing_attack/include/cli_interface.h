#pragma once
#include <string>
#include "../include/password_checker.h"
#include "../include/genetic_algorithm.h"

class CLIInterface {
private:
    PasswordChecker checker;
    GeneticAlgorithm ga;
    
    // Configuration
    std::string default_password;
    std::string charset;
    size_t population_size;
    double mutation_rate;
    double selection_rate;
    int max_generations;
    
public:
    CLIInterface();
    
    // Main interface loop
    void run();
    
    // Menu options
    void showMainMenu();
    void setPassword();
    void configureParameters();
    void runAttack();
    void runAggressiveAttack();
    void demonstrateTimingLeak();
    void advancedSettings();
    void showHelp();
    
    // Utility functions
    void printBanner();
    void printConfiguration();
    bool validatePassword(const std::string& password);
    std::string getValidPassword();
}; 