#ifndef SAFE_PRINT_HPP
#define SAFE_PRINT_HPP

#include <string>
#include <mutex>

//extern tells other files the mutex exists somewhere else:
extern std::mutex cout_mutex; 

void safe_print(const std::string& msg);

#endif //SAFE_PRINT_HPP