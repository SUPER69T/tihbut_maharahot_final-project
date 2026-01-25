//threaded timeout timer: (our alternative to using - setsockopt() timeout options...):
#include "threaded_t_timer.hpp"
#include <Network_Exception.hpp>
#include <atomic> //<time_point>start_time + <bool>active + load().

//constructor:
//launching a background thread that runs in parralel to the caller:
threaded_t_timer::threaded_t_timer( const std::string process_name, std::chrono::seconds timeout, const int check_interval_ms)
    : timeout_duration(timeout), interval(check_interval_ms){
        //first clock initialition:
        start_time = std::chrono::steady_clock::now();

        //a lambda function inside the timer's constructor:
        //---
        watcher = std::thread([this](){ //:
            //'this' inside the lambda's parameters is a shallow copy of a pointer to the class's(threaded_t_timer) object itself -
            // - this is done in order to gain access to the object's fields within the lambda expression.
            //
            //bad practice: catching the object by a reference inside the lambda is bad practice - 
            //because it captures the local variables OF THE CONSTRUCTOR, which get destroyed when it -
            //finishes constructing the object, leaving the thread with "dangling references" to garbage memory.
            //
            while(active){ // =
                auto now = std::chrono::steady_clock::now();//:
                //auto is easier than: "std::chrono::time_point<std::chrono::steady_clock>"...
                auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - start_time.load());//:
                //duration_cast<>() = a function that converts std::chrono::duration types from one to another.
                //load() = atomically retrieve the current value from an atomic variable.

                if(elapsed >= timeout_duration){
                    throw Socket_Exception("send_all() function in handle_client.cpp.", 0); //:
                    //passing 0 as an errno-field for it not to print the wrong type of error... 
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(interval));
            }
        });
        //---
    }
//
//destructor:
threaded_t_timer::~threaded_t_timer(){
    //telling the thread to stop:
    active = false;
    //waiting for the thread object to exit cleanly:
    if(watcher.joinable()) watcher.join(); //:
    //joinable() = a boolean function that checks whether the thread can be joined...(self-explanatory).
}
//
//
void threaded_t_timer::reset_timer(){
//this overwrites the thread's 'elapsed' back to 0:
start_time = std::chrono::steady_clock::now();
}
//