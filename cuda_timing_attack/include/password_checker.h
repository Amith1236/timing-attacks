#pragma once
#include <string>
#include <chrono>

class PasswordChecker {
private:
    std::string target_password;
    
public:
    PasswordChecker(const std::string& password);
    
    // Vulnerable password checker with timing leak
    bool checkPassword(const std::string& attempt);
    
    // Get timing measurement for password attempt
    double measureCheckTime(const std::string& attempt, int iterations = 100);
    
    // Get target password length
    size_t getPasswordLength() const;
    
    // Get target password (for verification)
    std::string getTargetPassword() const;
}; 