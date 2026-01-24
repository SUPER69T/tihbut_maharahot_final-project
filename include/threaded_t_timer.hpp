#ifndef THREADED_T_TIMER_HPP
#define THREADED_T_TIMER_HPP

#include <chrono>
#include <string>
#include <thread>
#include <atomic>

class timeout_timer{
private:
    std::thread watcher_thread;
    std::string name;
    
public:
    //Starts a background thread immediately:
    timeout_timer(std::chrono::seconds timeout, std::string timer_name);
    ~timeout_timer(); // Ensures thread joins/detaches safely
};

#endif //THREADED_T_TIMER_HPP