#include "../include/password_checker.h"
#include <algorithm>
#include <numeric>
#include <vector>

PasswordChecker::PasswordChecker(const std::string& password) 
    : target_password(password) {
}

bool PasswordChecker::checkPassword(const std::string& attempt) {
    // VULNERABLE: Early exit on first mismatch
    // This creates timing differences that can be exploited
    
    if (attempt.length() != target_password.length()) {
        return false;
    }
    
    // Compare character by character with early exit
    for (size_t i = 0; i < target_password.length(); ++i) {
        if (attempt[i] != target_password[i]) {
            return false;  // Early exit - timing leak!
        }
    }
    
    return true;
}

double PasswordChecker::measureCheckTime(const std::string& attempt, int iterations) {
    std::vector<double> times;
    times.reserve(iterations);
    
    for (int i = 0; i < iterations; ++i) {
        auto start = std::chrono::high_resolution_clock::now();
        checkPassword(attempt);
        auto end = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
        times.push_back(duration.count());
    }
    
    // Return median time for stability
    std::sort(times.begin(), times.end());
    if (iterations % 2 == 0) {
        return (times[iterations/2 - 1] + times[iterations/2]) / 2.0;
    } else {
        return times[iterations/2];
    }
}

size_t PasswordChecker::getPasswordLength() const {
    return target_password.length();
}

std::string PasswordChecker::getTargetPassword() const {
    return target_password;
} 