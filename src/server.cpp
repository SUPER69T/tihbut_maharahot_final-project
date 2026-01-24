//Includes:
//-----
//socket:
#include <arpa/inet.h>
#include <sys/socket.h>
//

//POSIX + thread:
#include <unistd.h>
#include <chrono>
#include <thread>
//

//IO:
#include <iostream>
//

//ADTs + primitives:
#include <string>
#include <cstring>
#include <vector>
//

//---
#include <cerrno> //provides errno:
//errno: a modifiable lvalue(locator value*) of int-type that acts as a macro that stores an integer - 
//value representing the last error that occurred during a system call or library function call. 
//
//lvalue: an expression that identifies a persistent object or a memory location and therefore has an address.
//---

//project specific:
#include <Network_Exception.hpp>
#include <Item.hpp>
#include <InventoryManager.hpp>
#include <handle_client.hpp>
#include <safe_print.hpp>
#include <timeout_timer.hpp>
//
//-----


//gemini helped a lot explaining the:
//1. socket creation.
//2. multi-threading control.
//3. consturcting most of the Network_Exception handling system.

//argc: argument count, argv: argument vector.
int main(int argc, char *argv[]){ //argv[program_path[0], Port[1], maxclients[2]].
    
    //immediately starting the first timeout timer:
    timeout_timer timer(std::chrono::seconds(20), "server startup timer."); 
    //

    //default arguments:
    int prt = 8080;
    int listen_counter = 20;
    //

    if(argc < 2){ //argc = 1.
        std::cerr << "\n" << std::endl;
        return 1;
    }
    else if(argc < 3){ //argc = 2.
        std::cerr << "Specify: port, maxclients.\n" << std::endl;
        return 1;
    }
    else if(argc < 4){ //argc = 3.
        prt = std::stoi(argv[1]);
        listen_counter = std::stoi(argv[2]);
    }
    else{ //3 < argc.
        std::cerr << "Too many arguments specified.\n" << std::endl;
        return 1;
    }

    //Creating an InventoryManager instance:
    std::vector<Store::Item> items_vec = {Store::Item(1, "Camera"),
    Store::Item(2, "Tripod"), Store::Item(3, "Laptop"),
    Store::Item(4, "Projector"), Store::Item(5, "Microphone"),
    Store::Item(6, "Speaker"), Store::Item(7, "HDMI_Cable"),
    Store::Item(8, "Ethernet_Cable"), Store::Item(9, "Keyboard"),
    Store::Item(10, "Mouse"), Store::Item(11, "Monitor"),
    Store::Item(12, "USB_Hub"), Store::Item(13, "Power_Bank")};

    Store::InventoryManager items(items_vec);


    
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

    //3: (binding to the socket):
    //-----
    try{
        //bind input structure:
        //bind(socket_descriptor, pointer_to_struct, size_of_struct)
        if(bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
            throw Bind_Exception("listening-socket binding failed.", errno); 
            //types of bind failures:
            //EADDRINUSE (Port ddress already in use): The most common error. Another process is already using port 8080.
            //EACCES (Permission denied): You tried to bind to a "privileged" port (0–1023) without root/administrator privileges.
            //EBADF: The sockfd you provided is not a valid file descriptor (e.g., the socket() call failed earlier).
            //EINVAL: The socket is already bound to an address.
        }
    } 
    catch (const Bind_Exception& e){ //:this is the earliest moment in the code where we might start throwing exceptions.
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


    //4. (listening to clients):
    //-----
    listen(server_fd, listen_counter); //limiting listens to the counter.
    safe_print("listening on port - " + std::to_string(prt) + "...");
    //-----

    //probably as good as any time to set the first main-timeout end-point:
        
    //

    //5. (accepting a connection): 
    //-----
    //Acceptting and handling a client's connection:
    while(true){
        //Accepting a new client's connection:
        int client_fd = accept(server_fd, nullptr, nullptr); //:
        //
        
        // Creating a new thread in order to handle the client:
        std::thread(handle_client, client_fd, std::ref(items)).detach(); //
        //*Note - this type of manual socket-opening technique we use here is called "Blocking-socket opening".
        //the reason it is discouraged(compared to non-Blocking alternatives like using select()/poll()/epoll()) is because - 
        //every operation we do on a single socket(read/write...) haults the thread, thus only enabling the creation of a single -
        //socket on each thread, making it much slower and inefficient for robust programs with many concurrent clients to handle.
    }

    //6. (closing the server socket):
    //-----
    safe_print("Closing the server in:");
    std::this_thread::sleep_for(std::chrono::seconds(1));
    safe_print("3...");
    std::this_thread::sleep_for(std::chrono::seconds(1));
    safe_print("2...");
    std::this_thread::sleep_for(std::chrono::seconds(1));
    safe_print("1...\ngoodbye! <O_O>");
    std::this_thread::sleep_for(std::chrono::seconds(1));
    close(server_fd);
    return 0;
    //-----
}