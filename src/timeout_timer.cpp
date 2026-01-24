#include "looping_t_timer.hpp"
#include <Network_Exception.hpp>
#include <chrono>
#include <string>


looping_t_timer::looping_t_timer(const std::chrono::seconds& timeout, const std::string& timer_name)
    : duration(timeout), name(std::move(timer_name)){ //EXPLAIN LATER: std:move.
    // Calculating the deadline once at construction:
    deadline = std::chrono::steady_clock::now() + timeout;
}

// Call this at the start of any method to enforce timeout
void looping_t_timer::check_timeout() const{
    if (std::chrono::steady_clock::now() > deadline){
        throw Timeout_Exception("Timer '" + name + "' has expired.", errno);
    }
}

void looping_t_timer::reset() {
    //the deadline is refreshed to "now + original duration":
    deadline = std::chrono::steady_clock::now() + duration;
}