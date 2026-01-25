//a thread-safe list of clients:
#ifndef T_CLIENTS_LIST_HPP
#define T_CLIENTS_LIST_HPP

#include <vector>
#include <mutex>
#include <condition_variable>

class t_clients_list{
        private:
            //fields:  
            std::mutex mtx;
            std::condition_variable cv; 
            std::vector<std::pair<std::string, bool>> clients_list; //vectors support RAII.
            //

        public:
            //constructors:
            t_clients_list(const size_t& size); //empty vector constructor.
            t_clients_list(const std::vector<std::pair<std::string, bool>>, const size_t& size); 

            //public methods:
            bool add_client(std::string client_name);
            bool remove_client(std::string client_name);
            //
    };

#endif //T_CLIENTS_LIST_HPP