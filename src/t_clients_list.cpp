#include "t_clients_list.hpp"
#include <string>
#include <vector>
#include <mutex>

//constructors:
//-----
t_clients_list::t_clients_list(const size_t& size) : clients_list(size){ //size-only constructor.
    std::lock_guard<std::mutex> lock(mtx);
    //assigning - 0 as a default client_fd for each client-pair's first position, and a default name at the second position:
    for(size_t i = 0 ; i < size ; i++){
        clients_list[i] = std::make_unique<client_info>(0, "client -" + std::to_string(i));
    }
} 

t_clients_list::t_clients_list(std::vector<std::unique_ptr<client_info>> vec, const size_t& size) : clients_list(size){ //vector-copying constructor(just in case...).
    std::lock_guard<std::mutex> lock(mtx);
    for(size_t i = 0 ; i < size ; i++){
        if(vec[i]){
            clients_list[i] = std::make_unique<client_info>(vec[i]->fd, vec[i]->name, vec[i]->is_timed_out == true);
        }
        else{
            clients_list[i] = std::make_unique<client_info>(0, "client -" + std::to_string(i));
        }
    }
}
//-----

//public methods:
//-----
//---
bool t_clients_list::contains(const std::string client_name){
    std::lock_guard<std::mutex> lock(mtx); 
    for(auto& p : this->clients_list){
        if(p && p->name == client_name) return true;
    }
    return false;
}

std::string t_clients_list::add_client(const int client_fd){
    std::lock_guard<std::mutex> lock(mtx); 
    std::string default_name = "";
    for(auto& p : this->clients_list){ 
        if(p && p->fd == 0){
            p->fd = client_fd;
            default_name = p->name;
            break;   
        }
    }
    return default_name;
}

bool t_clients_list::add_client(const int client_fd, const std::string client_name){
    std::lock_guard<std::mutex> lock(mtx); 
    for(auto& p : this->clients_list){ //updating pre-existing names to a new name.
        if(p && p->fd == client_fd){
            p->name = client_name;
            return true;
        }
    }
    for(auto& p : this->clients_list){ //adding a new client in case there is no pre-existing.
        if(p && p->fd == 0){
            p->fd = client_fd;
            p->name = client_name;
            return true;
        }
    }
    return false;
}
//---

//---
bool t_clients_list::remove_client(const int client_fd){
    std::lock_guard<std::mutex> lock(mtx); 
    for(auto& p : this->clients_list){
        if(p && p->fd == client_fd){
            p->fd = 0;
            p->name = "";
            p->is_timed_out = false;
            return true;
        }
    }
    return false;
}

bool t_clients_list::remove_client(const std::string client_name){
    std::lock_guard<std::mutex> lock(mtx); 
    for(auto& p : this->clients_list){
        if(p && p->name == client_name){
            p->fd = 0;
            p->name = "";
            p->is_timed_out = false;
            return true;
        }
    }
    return false;
}
//---
//-----

