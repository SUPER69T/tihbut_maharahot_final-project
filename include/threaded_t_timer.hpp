//threaded timeout timer:
#ifndef THREADED_T_TIMER_HPP
#define THREADED_T_TIMER_HPP

#include <chrono>
#include <string>
#include <thread>
#include <atomic>

class threaded_t_timer {
private:
    std::string process_name;
    std::chrono::seconds timeout_duration;
    int interval; //allows extra modification.
    
    // Tracks the current "start" of the countdown
    std::atomic<std::chrono::steady_clock::time_point> start_time;
    std::atomic<bool> active{true};
    std::thread watcher;

public:
    //constructor + destructor:
    threaded_t_timer(const std::string process_name, std::chrono::seconds timeout, const int check_interval_ms);
    ~threaded_t_timer(); //ensures the thread joins/detaches safely.
    //
    void reset_timer(); //updates start_time to "now".
};

#endif
