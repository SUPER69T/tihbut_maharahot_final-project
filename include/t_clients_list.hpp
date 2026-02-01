//a thread-safe list of clients:
#ifndef T_CLIENTS_LIST_HPP
#define T_CLIENTS_LIST_HPP

#include <vector> //main list(contiguous memory).
//we also allocate a <pair>-type for each client inside the vector.
#include <mutex>
#include <condition_variable>

class t_clients_list{
        private:
        //fields:  
        std::mutex mtx;
        std::condition_variable cv; 
        std::vector<std::pair<int, std::string>> clients_list; //vectors support RAII, so no explicit destructor needed.
        //

        public:
        //constructors:
        t_clients_list(const size_t& size); //empty vector constructor.
        t_clients_list(std::vector<std::pair<int, std::string>> vec, const size_t& size); 

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