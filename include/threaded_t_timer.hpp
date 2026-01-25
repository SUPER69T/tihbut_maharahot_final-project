//threaded timeout timer:
#ifndef THREADED_T_TIMER_HPP
#define THREADED_T_TIMER_HPP

#include <chrono>
#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>

class threaded_t_timer {
private:
    //4 construction inputs:
    std::string process_name;
    int target_socket;
    std::chrono::seconds timeout_duration;
    int interval; //allows extra modification.
    //

    // Tracks the current "start" of the countdown
    std::atomic<std::chrono::steady_clock::time_point> start_time;
    std::atomic<bool> active{true}; //used in the check-condition for the running timer-thread.
    bool expired = false; //used to check if the timeout was reached.
    std::thread watcher;
    std::mutex mtx;             
    std::condition_variable cv;
    //
    
public:
    //constructor + destructor:
    threaded_t_timer(const std::string process_name, int socket_fd, std::chrono::seconds timeout, const int check_interval_ms);
    ~threaded_t_timer(); //ensures the thread joins/detaches safely.
    //
    std::string const get_p_name(){return this->process_name;}
    void check_and_throw(); //updates start_time to "now".
    void reset_timer();
    bool is_expired(){return this->expired;}; //this check is suppose to be done by the caller.
};

#endif
