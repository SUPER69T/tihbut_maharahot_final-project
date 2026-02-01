#include "t_clients_list.hpp"
#include <string>
#include <vector>
#include <mutex>

//constructors:
//-----
t_clients_list::t_clients_list(const size_t& size): clients_list(size){ //size-only constructor.
    //assigning - 0 as a default client_fd for each client-pair's first position, and a default name at the second position:
    for(size_t i = 0 ; i < size ; i++){
        clients_list[i].first = 0;
        clients_list[i].second = "client - " + std::to_string(i);
    }
} 

t_clients_list::t_clients_list(std::vector<std::pair<int, std::string>> vec, const size_t& size): clients_list(size){ //vector-copying constructor(just in case...).
    for(size_t i = 0 ; i < size ; i++){
        clients_list[i].first = vec[i].first;
        clients_list[i].second = vec[i].second;
    }
}
//-----

//public methods:
//-----
//---
bool t_clients_list::contains(const std::string client_name){
    for(auto& p : this->clients_list){
        if(p.second == client_name) return true;
    }
    return false;
}

std::string t_clients_list::add_client(const int client_fd){
    std::lock_guard<std::mutex> lock(mtx); 
    std::string default_name = "";
    for(auto& p : this->clients_list){ 
        if(p.first == 0){
            p.first = client_fd;
            default_name = p.second;
            break;
        }
    }
    return default_name;
}

bool t_clients_list::add_client(const int client_fd, const std::string client_name){
    std::lock_guard<std::mutex> lock(mtx); 
    bool added = false;
    for(auto& p : this->clients_list){ //updating pre-existing names to a new name.
        if(p.first == client_fd){
            p.second = client_name;
            added = true;
            break;
        }
    }
    if(added == false){
        for(auto& p : this->clients_list){ //adding a new client in case there is no pre-existing.
            if(p.first == 0){
                p.first = client_fd;
                p.second = client_name;
                added = true;
                break;
            }
        }
    }
    return added;
}
//---

//---
bool t_clients_list::remove_client(const int client_fd){
    std::lock_guard<std::mutex> lock(mtx); 
    bool removed = false;
    for(auto& p : this->clients_list){
        if(p.first == client_fd){
            p.first = 0;
            p.second = "";
            removed = true;
            break;
        }
    }
    return removed;
}

bool t_clients_list::remove_client(const std::string client_name){
    std::lock_guard<std::mutex> lock(mtx); 
    bool removed = false;
    for(auto& p : this->clients_list){
        if(p.second == client_name){
            p.first = 0;
            p.second = "";
            removed = true;
            break;
        }
    }
    return removed;
}
//---
//-----

