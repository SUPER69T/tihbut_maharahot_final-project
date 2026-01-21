//socket:
#include <arpa/inet.h>
#include <sys/socket.h>
//

//POSIX + thread:
#include <unistd.h>
#include <thread>
//

//IO:
#include <iostream>
#include <fstream> //::ofstream = used to read and write into files.
//

//ADTs + primitives:
#include <string>
#include <cstring>
#include <vector>
#include <unordered_map>
//

//cv + mtx:
#include <mutex>
#include <condition_variable>
//

//---
#include <cerrno> //provides errno:
//errno: a modifiable lvalue(locator value*) of int-type that acts as a macro that stores an integer - 
//value representing the last error that occurred during a system call or library function call. 
//
//lvalue: an expression that identifies a persistent object or a memory location and therefore has an address.
//---

//project specific:
#include "InventoryManager.hpp"
#include "Item.hpp"
#include <Network_Exception.hpp>
//

//gemini helped a lot explaining the:
//1. socket creation.
//2. multi-threading control.
//3. ...

//and it also helped consturcting most of the Network_Exception handling system.
//this is one of the only blocks we fully let it generate:

//{
//Thread-safe logging function that opens and saves into a file:
void log_error(const std::string& message) {
    static std::mutex log_mutex; // Shared across all threads
    std::lock_guard<std::mutex> lock(log_mutex); // Locks on entry, unlocks on exit

    std::ofstream log_file("server_errors.log", std::ios::app); //"server_errors.log" = the name of the logging file.
    //app = append modifier for the stream to write to the end of the file, regardless of the file's pointer position.
    if (log_file.is_open()) {
        log_file << "[LOG]: " << message << std::endl;
    } else {
        std::cerr << "Critical: Could not open log file!" << std::endl;
    }
}
//}


int main(int argc, char *argv[]){
    //set argc, argv checks.
    int prt = 8080;

    //1: (socket assigning):
    //-----
    int server_fd = socket(AF_INET, SOCK_STREAM, 0); //server-File-Descriptor(on linux OS).
    //domain: AF_INET(AF = address family) = Internet Domain (AF_INET): The standard for network sockets, 
    //using IP addresses and port numbers to communicate across different machines over protocols like TCP/IP(IPv4 in our case).
    //
    //SOCK_STREAM = a socket type that provides reliable, ordered, and error-checked data delivery -
    //through a connection-oriented byte stream, primarily using the TCP protocol.
    //
    //protocol: 0 = Default Protocol: Because you requested a stream socket over IPv4, the OS defaults to TCP (IPPROTO_TCP).

    
    //modified socket behavior to allow immediate reuse of the port(Gemini's implementation...):
    //-------
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    //-------
    //-----


    //2: (configuring the address):
    //-----
    sockaddr_in server_addr{};
    //structure layout of sockaddr_in (16-byte):
    // Offset  Member	   Type	                    Size       Purpose
    // 0	   sin_family  sa_family_t (uint16)	    2 bytes	   Address family (e.g., AF_INET)
    // 2	   sin_port	   in_port_t (uint16)	    2 bytes	   Port number (Big-Endian)
    // 4	   sin_addr	   struct in_addr (uint32)	4 bytes	   IPv4 Address (Big-Endian)
    // 8	   sin_zero	   char[8]	                8 bytes	   Padding (Must be all zeros)

    server_addr.sin_family = AF_INET; //Sets the address family to IPv4.
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); //INADDR_ANY = a constant representing the IPv4 wildcard address 0.0.0.0 -
    //When passed to bind(), it instructs the operating system to accept incoming packets destined for -
    //any local IP address assigned to the host's network interfaces.
    //
    //while htonl(Host To Network Long): Converts a 32-bit number from host to network byte order -
    //(from Little-Endian [our CPU's order] to Big-Endian[TCP's order])
    server_addr.sin_port = htons(prt); //Sets the port address to prt.
    //
    //while htons(Host To Network Short): Converts a 16-bit number from host to network byte order -
    //(from the CPU's order[Little-Endian in most CPUs{intel/AMD}] to the Network's order[Big-Endian in most IP types{TCP/UDP}]).
    //-----

    // 3: (binding to the socket):
    //-----
    try{
        //bind input structure:
        //bind(socket_descriptor, pointer_to_struct, size_of_struct)
        if(bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
            throw Bind_Exception("socket binding failed.", errno); 
            //types of bind failures:
            //EADDRINUSE (Port ddress already in use): The most common error. Another process is already using port 8080.
            //EACCES (Permission denied): You tried to bind to a "privileged" port (0–1023) without root/administrator privileges.
            //EBADF: The sockfd you provided is not a valid file descriptor (e.g., the socket() call failed earlier).
            //EINVAL: The socket is already bound to an address.
        }
    } 
    catch (const Bind_Exception& e){ //:this is the earliest moment in the code where we might start throwing exceptions.
        log_error(e.what()); //here we implement the thread-safe logging function that gemini has generated.
        //The manul errno checking(explained in - Network_Exception, line ~ 18):
        if(e.get_code() == 98){//Ideally this peace of code would be reused as boilerplate code but -
        //the spread out socket opening explanations were worth the extra space...   
            server_addr.sin_port = htons(prt + 1); //trying to reopen the socket on a higher port number.
            if(bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
                throw Bind_Exception("socket binding failed.", errno); //if this part of code throws an error or fails -
                //in any way, it should cause the program to hault until the "Timeout" timer runs out.
            }
        }
    }
    //-----


    // 4. listen – השרת מוכן לקבל חיבורים
    listen(server_fd, 1); // תור חיבורים בגודל 1

    std::cout << "Server waiting for client...\n";

    // 5. accept – קבלת חיבור מלקוח
    int client_fd = accept(server_fd, nullptr, nullptr);

    // 6. קבלת הודעה מהלקוח
    char buffer[1024];
    recv(client_fd, buffer, sizeof(buffer), 0);

    std::cout << "Received from client: " << buffer << std::endl;

    // 7. שליחת תשובה ללקוח
    const char* reply = "Hello from server";
    send(client_fd, reply, strlen(reply) + 1, 0);

    // 8. סגירת sockets
    close(client_fd);
    close(server_fd);

    return 0;
}






/*
int main(int argc, char *argv[]){
    if (argc < 4)
    while(true){

    }


// try{
//     Item& founditem = findItemById(itemId);
//     if(founditem.isAvailable()){
//     founditem.borrow(username);
//     }
//     std::lock_guard<std::mutex> unlock(mtx);
// }
// catch (const std::runtime_error& e){
//     std::cerr << e.what() << std::endl;
// }
// catch (const std::invalid_argument& e){
//     std::cerr << e.what() << std::endl;
// }
}
*/







/*
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

            std::lock_guard<std::mutex> lock(inventory_mutex);
            auto items = inventory.getAllItems();
            
            for (const auto& item : items) {
                std::string line = item.getName() + " " + std::to_string(item.getQuantity()) + "\n";
                send_all(client_fd, line);
            }
            send_all(client_fd, ".\n"); 
        }    
        
    }
}
*/