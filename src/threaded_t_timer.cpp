//threaded timeout timer: (our alternative to using - setsockopt() timeout options...):
#include "threaded_t_timer.hpp"
#include <unistd.h> //close().
#include <sys/socket.h> //shutdown().
#include <Network_Exception.hpp>
#include <atomic> //<time_point>start_time + <bool>active + load().
#include <mutex>
#include <condition_variable>

//constructor:
//launching a background thread that runs in parralel to the caller:
threaded_t_timer::threaded_t_timer(const std::string process_name, int socket_fd, std::chrono::seconds timeout, const int check_interval_ms)
    : process_name(process_name), target_socket(socket_fd), timeout_duration(timeout), interval(check_interval_ms){

    //first clock initialition:
    start_time = std::chrono::steady_clock::now();

    //a lambda function as each new timer-thread's process, all inside the timer's constructor:
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
            //signaling the timeout:
            {//extra scope to RAII the lock_guard.
                std::lock_guard<std::mutex> lock(mtx);
                expired = true;
            }//end of extra scope.

                //shutting down and closing the socket in case of a timeout:
                if (target_socket != -1) {
                    shutdown(target_socket, SHUT_RDWR); //shutdown tells the OS to stop any current I/O immediately.
                    close(target_socket); //close releases the file descriptor.
                }

            cv.notify_all(); // Wake up anyone waiting for this signal
            return; //returning ends the timer thread.
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(interval));
        }
        //here the thread exits quietly if the task is finished(caller exited the scope, which called the timer's destructor).
    });
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
    std::lock_guard<std::mutex> lock(mtx);
    start_time = std::chrono::steady_clock::now();
    expired = false; //resets the signal too.
}
//

//
void threaded_t_timer::check_and_throw(){
    std::unique_lock<std::mutex> lock(mtx);
    
    // We wait for the signal without a loop! 
    // This will throw the exception the moment the watcher notifies us.
    if (cv.wait_for(lock, std::chrono::milliseconds(1), [this]{return expired;})){
         //goes here if the timer ran out:
        throw Socket_Exception(process_name + "'s timeout_timer ran out.", 0); //:
        //passing 0 as an errno-field for it not to print the wrong type of error... */
    }
}
//
