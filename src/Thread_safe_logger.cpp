#include "Thread_safe_logger.hpp"
#include <iostream>


//constructor:
Thread_safe_logger::Thread_safe_logger(){
    worker = std::thread(&Thread_safe_logger::processMessages, this); //:
    //both initializing the process and feeding in the newly constructed static ThreadSafeLogger - object.
}
//

//destructor:
Thread_safe_logger::~Thread_safe_logger(){
    {
    std::lock_guard<std::mutex> lock(mtx);
    exit = true;
    }
    cv.notify_all(); //using 'notify_all()' plays a minimal role in here, might aswell have just used - 
    //notify_one(), since only a single class instance exists at all time in a singleton class.
    if(worker.joinable()) worker.join(); // waiting for the worker to finish printing the remaining logs.
};
//

//methods:
//-----
Thread_safe_logger& Thread_safe_logger::getInstance(){
    static Thread_safe_logger instance; //a thread-safe and lazy initialized ThreadSafeLogger-object.
    return instance;
}
//

void Thread_safe_logger::log(const std::string& msg){
    {
        std::lock_guard<std::mutex> lock(mtx);
        msgQueue.push(msg);
    }
    cv.notify_one();
}

void Thread_safe_logger::processMessages(){
    while(true){
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this] {return !msgQueue.empty() || exit;}); //when the cv wakes up it reclaims the mutex only if the queue is not empty or the 'exit' flag is 'true',
        //sending it to the next while loop which makes sure to 'cout' and empty the entire stack before returning or breaking the wrapper loop if the exit-flag is 'true':
        while(!msgQueue.empty()){
            std::cout << msgQueue.front() << "\n"; //flushing with every incoming message.
            msgQueue.pop();
        }
        if(exit) break;
    }
    std::cout << std::flush;
//-----
}


