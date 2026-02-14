//threaded timeout timer: (our alternative to using - setsockopt() timeout options...):
#include "threaded_t_timer.hpp"
#include <unistd.h> //close().
#include <sys/socket.h> //shutdown().
#include <Network_Exception.hpp>
#include <atomic> //<time_point>start_time + <bool>active + load().
#include <mutex>
#include <condition_variable>

//constructor:
//---
//launching a background thread that runs in parralel to the caller:
threaded_t_timer::threaded_t_timer(const int fd, const std::string client_name, t_clients_list& clients_list, std::chrono::seconds timeout_seconds, const int check_interval_ms)
    : fd(fd), client_name(client_name),
    clients_list(clients_list), 
    //assigning default timeout and interval:
    timeout((timeout_seconds.count() < 1 || 1200 < timeout_seconds.count()) 
          ? std::chrono::seconds(30) //default timeout(in seconds).
          : timeout_seconds), 
    interval((check_interval_ms < 5 || 5000 < check_interval_ms) 
          ? 50 ////default interval(in ms).
          : check_interval_ms)
    //
{
    //first time snapshot:
    start_time = std::chrono::steady_clock::now();
    //
    active = true;
    expired = false;

    //a lambda function as each new timer-thread's process, all inside the timer's constructor:
    //---
    watcher = std::thread([this](){ //:
        //'this' inside the lambda's parameters is a shallow copy of a pointer to the class's(threaded_t_timer) object itself -
        // - this is done in order to gain access to the object's fields within the lambda expression.
        //
        //bad practice: catching the object by a reference inside the lambda is bad practice - 
        //because it captures the THE CONSTRUCTOR's local variables, which get destroyed when it -
        //finishes constructing the object, leaving the thread with "dangling references" to garbage memory.
        //
        while(active && !expired){
            {
                std::lock_guard<std::mutex> lock(mtx);
                auto now = std::chrono::steady_clock::now();//:
                //auto is easier than: "std::chrono::time_point<std::chrono::steady_clock>"...
                auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - start_time.load());//:
                //duration_cast<>() = a function that converts std::chrono::duration types from one to another.
                //load() = atomically retrieve the current value from an atomic variable.

                if(elapsed >= timeout){
                    //shutting down and closing the socket in case of a timeout:
                    expired  = true;
                    if(this->clients_list.set_client_timed_out(this->fd)){ //:
                        //skips this block when: invalid client_info ptr / fd < 0 / or in case no client matches the given fd.
                        shutdown(this->fd, SHUT_RDWR); //shutdown tells the OS to stop any current I/O immediately(send/recv...).
                    }
                }
            }
            if(active && !expired){
                std::unique_lock<std::mutex> lock(mtx);
                cv.wait_for(lock, std::chrono::milliseconds(interval), [this]{return !active || expired;});
            }
        }
    });
} //here the constructor finished running, the watcher thread now runs independantly until either:
//1. handle_client went out of scope, triggering the threaded_t_timer-class's custom destructor to .join() - 
//the watcher-thread and then call the class's default destructor automatically, freeing all field memory naturally. 
//2. the timer runs out and a signal is given back to handle_client to throw a Timeout_Exception, which then makes -
//handle_client exit the scope and trigger the sequence of events in - (1.).
//---

//destructor:
//---
threaded_t_timer::~threaded_t_timer(){
    //telling the thread to stop:
    active = false;
    cv.notify_all(); //notifying the watcher thread to wake up.
    //waiting for the thread object to exit cleanly:
    if(watcher.joinable()) watcher.join(); //:
    //joinable() = a boolean function that checks whether the thread can be joined...(self-explanatory).
    //.join() sequence: the destructor is called->destructor waits for watcher-thread to finish running -
    //before continuing with the default destructor, freeing object fields, and allowing watcher access -
    //to garbage memory, or potentially causing a crash because the timer-object will get destroyed -
    //before the watcher-thread shuts-down->watcher finishes->default constructors run->happy ending.
}
//---

//Methods:
//---
void threaded_t_timer::reset_timer_or_throw(){
    std::lock_guard<std::mutex> lock(mtx);
    if(expired){
        throw Timeout_Exception(client_name + "'s timeout_timer ran out.", 0); //:
        //passing 0 as an errno-field for it not to print the wrong type of error... */
    }
    start_time = std::chrono::steady_clock::now();
}
//---
