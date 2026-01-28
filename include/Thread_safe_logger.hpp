#ifndef THREAD_SAFE_LOGGER_HPP
#define THREAD_SAFE_LOGGER_HPP


//our own alternative to C++23’s std::println(which is too- thread safe...):
#include <string>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <thread>

class Thread_safe_logger{

    private:
    std::queue<std::string> msgQueue;
    std::mutex mtx;
    std::condition_variable cv;
    bool exit = false;
    std::thread worker;
    
    //constructor:
    Thread_safe_logger();
    //

    //destructor:
    ~Thread_safe_logger();
    //

    public:
    
    //initialization of a singleton Thread_safe_logger-object - instance.
    static Thread_safe_logger& getInstance();

    //disallowing the use of copy-constructors:
    Thread_safe_logger(const Thread_safe_logger&) = delete;
    Thread_safe_logger& operator = (const Thread_safe_logger&) = delete;
    //

    //a thread-safe method to push incoming messages:
    void log(const std::string& msg);

    void processMessages();
};
#endif //THREAD_SAFE_LOGGER_HPP