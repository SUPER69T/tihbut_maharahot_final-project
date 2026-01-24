#include "timeout_timer.hpp"
#include <Network_Exception.hpp>
#include <chrono>
#include <string>


timeout_timer::timeout_timer(std::chrono::milliseconds& timeout, const std::string& timer_name)
    : name(std::move(timer_name)){
    // Calculating the deadline once at construction:
    deadline = std::chrono::steady_clock::now() + timeout;
}

// Call this at the start of any method to enforce timeout
void timeout_timer::check_timeout() const{
    if (std::chrono::steady_clock::now() > deadline){
        throw Timeout_Exception("Timer '" + name + "' has expired.", errno);
    }
}
