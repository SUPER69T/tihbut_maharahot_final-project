#include "safe_print.hpp"
#include <iostream>

std::mutex cout_mutex; //one mutex on a global scale.
//this could be a good place to implement a singleton-class - instance of a safe_print that can only - 
//hold a single instance at all time, ensuring safety on a global scale across all safe_print uses.

void safe_print(const std::string& msg) {
    std::lock_guard<std::mutex> lock(cout_mutex);
    std::cout << msg << std::endl;
}


