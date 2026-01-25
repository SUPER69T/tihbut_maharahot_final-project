#include "t_clients_list.hpp"
#include <string>
#include <vector>
#include <mutex>

//constructors:
t_clients_list::t_clients_list(const size_t& size): clients_list(size){ //empty vector constructor.
    //assigning a default number to each client position and a boolean that represens whether this position is free:
    for(size_t i = 0 ; i < size ; i++){
        clients_list[i].first = "client - " + std::to_string(i);
        clients_list[i].second = false;
    }
} 

t_clients_list::t_clients_list(const std::vector<std::pair<std::string, bool>> vec, const size_t& size): clients_list(size){
    for(size_t i = 0 ; i < size ; i++){
        clients_list[i].first = vec[i].first;
        clients_list[i].second = vec[i].second;
    }
}
//

//public methods:
bool t_clients_list::add_client(std::string client_name){
    std::lock_guard<std::mutex> lock(mtx); 
    bool added = false;
    for(auto& p : this->clients_list){
        if(p.second == false || p.first == client_name){ //also updates a pre-existing name.
            p.first = client_name;
            p.second = true;
            added = true;
        }
    }
    return added;
}


bool t_clients_list::remove_client(std::string client_name){
    std::lock_guard<std::mutex> lock(mtx); 
    bool removed = false;
    for(auto& p : this->clients_list){
        if(p.first == client_name){
            p.first = "";
            p.second = false;
            removed = true;
        }
    }
    return removed;
}
//

