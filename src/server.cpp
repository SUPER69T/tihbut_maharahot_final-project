#include <iostream>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string>
#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>
#include "InventoryManager.hpp"


// Read one line until '\n'
bool recv_line(int fd, std::string& out) {
    out.clear();
    char c;
    while (true) {
        ssize_t n = recv(fd, &c, 1, 0);
        if (n <= 0) return false;
        if (c == '\n') break;
        out.push_back(c);
    }
    return true;
}

// Send the entire message
void send_all(int fd, const std::string& msg) {
    size_t sent = 0;
    while (sent < msg.size()) {
        ssize_t n = send(fd, msg.data() + sent, msg.size() - sent, 0);
        if (n <= 0) return;
        sent += n;
    }
}


void handle_client(int client_fd){
    bool is_authenticated=false; 
    std::string username;   
    
    while(true){
        bool check_username=true;
        std::string line;
        if(!recv_line(client_fd, line)) break;
        std::string command;
        std::string arg;
        size_t space_pos=line.find(' ');
        if(space_pos!=std::string::npos){
            command=line.substr(0,space_pos);
            arg=line.substr(space_pos+1);
        }else{
            command=line;
        }

        if (command=="HELLO"){
                username=arg;
                if(username.empty()){
                    send_all(client_fd, "ERR PROTOCOL missing_username\n");
                    check_username=false;
                    continue;
                }
                
                for (char c : username){
                    if(!isalpha(c)){
                        is_authenticated=false;
                        send_all(client_fd, "ERR PROTOCOL invalid_username\n");
                        check_username=false;
                        break;
                       
                    }
                }
                if(check_username){
                    is_authenticated=true;
                    send_all(client_fd, "OK HELLO\n");
                }
                
        }else if(!is_authenticated){
            send_all(client_fd, "ERR STATE not_authenticated\n");
           

            
            }
            else if(command=="LIST"){
                send_all(client_fd, "OK LIST\n");
                
                
                
            
     
        }
    }
}




























int main(int argc, char **argv){
/*
try{
    Item& founditem = findItemById(itemId);
    if(founditem.isAvailable()){
    founditem.borrow(username);
    }
    std::lock_guard<std::mutex> unlock(mtx);
}
catch (const std::runtime_error& e){
    std::cerr << e.what() << std::endl;
}
catch (const std::invalid_argument& e){
    std::cerr << e.what() << std::endl;
}
*/
}