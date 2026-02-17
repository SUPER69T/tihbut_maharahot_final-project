//threaded timeout timer:
#ifndef THREADED_T_TIMER_HPP
#define THREADED_T_TIMER_HPP

#include <chrono>
#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include "t_clients_list.hpp"

class threaded_t_timer{
    private:
    //4 construction inputs:
    int fd;
    std::string client_name;
    t_clients_list& clients_list;
    std::chrono::seconds timeout;
    int interval; //allows extra modification.
    //

    //tracks the current "start" of the countdown
    std::atomic<std::chrono::steady_clock::time_point> start_time;
    std::atomic<bool> active{true}; //used in the check-condition for the running timer-thread.
    bool expired = false; //used to check if the timeout was reached.
    std::mutex mtx;             
    std::condition_variable cv;
    std::thread watcher; //should be initialized last, after all other fields get constructed -
    //to ensure the watcher thread won't try accessing uninitialized object fields.
    //
        
    public:
    //constructor + destructor:
    threaded_t_timer(const int fd, const std::string client_name, t_clients_list& clients_list, std::chrono::seconds timeout_seconds, const int check_interval_ms);
    ~threaded_t_timer(); //ensures the thread joins/detaches safely.
    //

    //Methods:
    void reset_timer_or_throw();
    void update_name(std::string& new_client_name);
    inline std::string const get_p_name(){return this->client_name;}
    inline bool is_expired(){return this->expired;}; //allows the caller to manually check the state of the timer.
    //
};

#endif
