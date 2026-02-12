//a thread-safe list of clients:
#ifndef T_CLIENTS_LIST_HPP
#define T_CLIENTS_LIST_HPP

#include <vector> //main list(contiguous memory).
//we also allocate a "client_info"-struct for each client inside the vector.
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <string>

struct client_info{
    int fd;
    std::string name;
    std::atomic<bool> is_timed_out;
    
    client_info(int f = 0, std::string n = "", bool t = false) 
    : fd(f), name(n){
        is_timed_out.store(t); //using .store() is the safest way to set an atomic's value.
    }
};

class t_clients_list{
        private:
        //fields:  
        std::mutex mtx;
        std::condition_variable cv; 
        std::vector<std::unique_ptr<client_info>> clients_list; //vectors support RAII, so no explicit destructor needed.
        //using the unique_ptr makes the non-copyable struct a moveable pointer that also supports RAII.
        //

        public:
        //constructors:
        t_clients_list(const size_t& size); //empty vector constructor.
        t_clients_list(std::vector<std::unique_ptr<client_info>> vec, const size_t& size); 

        //public methods:
        //-----
        bool contains(const std::string client_name);
        //---
        //initializes a client with client_fd:
        std::string add_client(const int client_fd);
        //adds a new name \ updates a pre-existing name based on a given client_fd:
        bool add_client(const int client_fd, const std::string client_name);
        //---

        //---
        //overloading this function so that both a client_fd or a client_name are accepted:
        bool remove_client(const int client_fd);
        bool remove_client(const std::string client_name); 
        //---
        //-----
    };

#endif //T_CLIENTS_LIST_HPP