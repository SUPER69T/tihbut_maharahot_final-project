#ifndef NETWORK_EXCEPTION_H
#define NETWORK_EXCEPTION_H

#include <stdexcept>
#include <string>
#include <cstring> //used for strerror_r.
//sterror is an awsome C function that returns the text description of the error types. it requires an error-code.

//used in log_error():
//--- 
#include <mutex>
#include <iostream>
#include <fstream> //::ofstream = used to read and write into files.
#include <filesystem> //::path - used for: __FILE__ + .parent_path().
//---

#include "thread_safe_logger.hpp"

//gemini's help for making a hirarchy-exceptions structure:
//this exception class is used for server-network-sided exception throwing, meaning the client does not get access to the exception-logging file.
//{
//Base Network Exception:
//children: Bind_Exception, Timeout_Exception.
class Network_Exception : public std::runtime_error{
    protected:
        int error_code; //used to save-in-place the error code errno returns, a good idea for multi-threading exception implementation.

    public:
        //constructor:
        explicit Network_Exception(const std::string& msg, int err) //input: (message + the compiler's error code).
        : std::runtime_error(format_message(msg, err)), //:
        error_code(err){
            log_error(); //logging the message on every Network_Exception instanciation. 
            thread_safe_logger::getInstance().log(this->what()); //printing the error message.
        } 
        //   

        //virtual destructor:
        virtual ~Network_Exception() noexcept = default;
        //
        
        //implemented mainly for the practice itself:
        int get_code() const noexcept {return error_code;} //a method used for manual errno if-condition checking in a "catch" block where the exception is -
        //thrown in order to apply an alternative operation rather than Network_Exception's standard handling(implemented in - server.cpp, line ~ 136).
        
    private:
        //this is one of the only blocks we fully let gemini generate:
        //{
        //Thread-safe logging function that opens and saves into a file:
        void log_error() const noexcept{ 
            static std::mutex log_mutex; // Shared across all threads
            std::lock_guard<std::mutex> lock(log_mutex); //Locks on entry, unlocks on exit.

            //---
            //now this is a special piece of code gpt showed us to make sure log_file gets created next to the source file(post-compilation) -
            //on linux OS distros, instead of the pwd, which happens to be a much harder task than we initially speculated.
            //__FILE__ = a compile-time macro containing the path of the source file as seen by the compiler.
            //.parent_path() = a method from the std::filesystem::path that returns the parent directory that contains the current path.
            //this "trick" is a bad practice of how c++ should be handled as it leaks build-system structure into runtime behavior.
            std::filesystem::path source_dir = std::filesystem::path(__FILE__).parent_path();
            //---

            std::ofstream log_file(source_dir / "server_errors.log", std::ios::app); //"server_errors.log" = the name of the logging file.
            //app = append modifier for the stream to write to the end of the file, regardless of the file's pointer position.
            if (log_file.is_open()) {
                log_file << "[LOG]: " << what() << "\n"; 
            } else {
                std::cerr << "Critical: Could not open log file!"; //cerr does not require explicit flushing because it is unbuffered...
            }
        }
        //}

        //Helper to handle thread-safe string conversion:
        static std::string format_message(const std::string& msg, const int err){
            char buf[256];
            auto result = strerror_r(err, buf, sizeof(buf)); //strerror_r is a thread-safe alternative to strerror.
            //note on strerror_r: returns an int(0/error code) on XSI, returns char*(a pointer to the message) on GNU:
            return std::string(get_error_string(result, buf)) + ": " + msg;
        }
        //gemini's work:
        //---
        //Helper for the GNU version (returns the pointer directly)
        static const char* get_error_string(char* gnu_result, char* /*buf*/){ //buf is unused.
            return gnu_result;
        }
        // Helper for the XSI version (returns the buffer we provided)
        static const char* get_error_string(int /*xsi_result*/, char* buf){ //xsi is unused.
            return buf;
        }
        //---
    };

//a Network_Exception's child- "Bind" Error:
class Bind_Exception : public Network_Exception{
    public:
        explicit Bind_Exception(const std::string& msg, const int err) 
        : Network_Exception("Bind Error: " + msg, err){}
    };

//a Network_Exception's child- "Timeout" Error:
class Timeout_Exception : public Network_Exception{
    public:
        explicit Timeout_Exception(const std::string& msg, const int err) 
        : Network_Exception("Timeout Error on: " + msg, err){}
    };

//a Network_Exception's child- "Socket" Error:
class Socket_Exception : public Network_Exception{
    public:
        explicit Socket_Exception(const std::string& msg, const int err) 
        : Network_Exception("Socket Error: " + msg, err){}
    };

#endif //NETWORK_EXCEPTION_H