#include "threaded_t_timer.hpp"
#include <iostream>
#include <Network_Exception.hpp>



threaded_t_timer::threaded_t_timer(std::chrono::seconds timeout, std::string timer_name) 
    : name(std::move(timer_name)){ //EXPLAIN LATER: std:move.
    
    //launching a background thread that runs in parralel to other processes:
    watcher_thread = std::thread([timeout, this](){//all of this is the lambda's function.
        std::this_thread::sleep_for(timeout);

        std::cerr << "\n[FATAL] Timer '" << name << "' expired. Force quitting..." << std::endl;//EXPLAIN
        std::exit(1);
    });
    
    watcher_thread.detach();
}
    threaded_t_timer::~threaded_t_timer(){
    }