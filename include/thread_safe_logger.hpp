#ifndef THREAD_SAFE_LOGGER_HPP
#define THREAD_SAFE_LOGGER_HPP


//our own alternative to C++23’s std::println(which is too- thread safe...):
#include <string>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <thread>

class thread_safe_logger{

    private:
    std::queue<std::string> msgQueue;
    std::mutex mtx;
    std::condition_variable cv;
    bool exit = false;
    std::thread worker;
    
    //constructor:
    thread_safe_logger();
    //

    //destructor:
    ~thread_safe_logger();
    //

    public:
    
    //initialization of a singleton Thread_safe_logger-object - instance.
    static thread_safe_logger& getInstance();

    //disallowing the use of copy-constructors:
    thread_safe_logger(const thread_safe_logger&) = delete;
    thread_safe_logger& operator = (const thread_safe_logger&) = delete;
    //

    //a thread-safe method to push incoming messages:
    void log(const std::string& msg);

    void processMessages();
};
#endif //THREAD_SAFE_LOGGER_HPP