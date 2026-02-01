//Includes:
//-----
//socketing:
#include <arpa/inet.h>
#include <sys/socket.h>
//

//POSIX + threading:
#include <unistd.h>
#include <thread>
//

//---
#include <chrono> //using here:
//std::chrono::seconds = a time duration type, represents time interval with a precision of one second.
//std::chrono::steady_clock::now() = a monotonic clock, perfect for measuring time intervals and benchmarking.
//---
//

//IO:
#include <iostream>
//

//ADTs + primitives:
#include <string>
#include <cstring>
#include <vector>
//---
#include <cerrno> //provides errno:
//errno: a modifiable lvalue(locator value*) of int-type that acts as a macro that stores an integer - 
//value representing the last error that occurred during a system call or library function call. 
//
//lvalue: an expression that identifies a persistent object or a memory location and therefore has an address.
//---
//

//locking:
//#include <mutex>
//#include <condition_variable>
//
//project specific:
#include "Network_Exception.hpp"
#include "handle_client.hpp"
#include "thread_safe_logger.hpp"
//#include "threaded_t_timer.hpp"
#include "t_clients_list.hpp"
#include "handle_client.hpp"
//-
//don't know why "Store" namespace refuses to be included, instead we're forced into including these two bastards that hate co-operating: 
#include "Item.hpp"
#include "InventoryManager.hpp"
//-
//-----

//-----
//Disclaimer:
//gemini did help quite a bit with explaining the:
//socket creation,
//multi-threading control,
//Network_Exception handling system,
//threaded_t_timer implementation,
//and much more syntax/conventions/rules of thumb...
//
//it did also help generate quite a lot of examples that were used to study and implement some of -
//this pretty ambitious project that did take quite a lot of time to build from scratch.
//
//but the most important part is that IT WAS used to study and prepare for the exam, which I(leon) - 
//personally believe would otherwise be impossible to truly grasp the concepts of this awful coding language.
//-----


//just for fun...:
int close_main(const int& err){ 
    thread_safe_logger::getInstance().log("Closing the server in:");
    std::this_thread::sleep_for(std::chrono::seconds(1));
    thread_safe_logger::getInstance().log("3...");
    std::this_thread::sleep_for(std::chrono::seconds(1));
    thread_safe_logger::getInstance().log("2...");
    std::this_thread::sleep_for(std::chrono::seconds(1));
    thread_safe_logger::getInstance().log("1...\ngoodbye! <O_O>");
    std::this_thread::sleep_for(std::chrono::seconds(1));
    return err;
}
/*
bool is_timeout_reached(threaded_t_timer& timer){
    std::mutex mtx;
    std::unique_lock<std::mutex> ul(mtx);
    std::condition_variable cv;
    if(cv.wait(ul, [&timer](){timer.is_expired();}) == true){
        throw Socket_Exception(timer.get_p_name() + "'s timeout_timer ran out.", 0); //:
        //passing 0 as an errno-field for it not to print the wrong type of error... 
    }
}
*/
/*
bool is_timeout_reached(threaded_t_timer timer){
    if(!timer.is_expired()) return true;
    throw Socket_Exception(timer.get_p_name() + "'s timeout_timer ran out.", 0); //:
    //passing 0 as an errno-field for it not to print the wrong type of error... 
}
*/

//-----
//MAIN STARTS HERE:
//argc: argument count, argv: argument vector.
int main(int argc, char *argv[]){ //argv[program_path[0], Port[1], maxclients[2]].

    thread_safe_logger& logger = thread_safe_logger::getInstance(); //initialization of the one and only -
    //singleton - Thread_safe_logger-object instance for the entire program's life-time.

    int server_fd;

    try{ //the entire main is inside a try block for catching the Timeout_Exception.

        //immediately starting the first timeout timer:
        /////////threaded_t_timer timer1("main timer", std::chrono::seconds(20), 100); 
        //

        //default aegv parameters:
        int prt = 5555; //could also use - 8080.
        int clients_limit = 3; //max amount of connections that can be astablished. important because we are using blocking socketing.
        //

        if(argc < 1){ //argc = 1.
            std::cerr << "\n" << std::endl;
            return close_main(-1);
        }
        else if(argc < 2){ //argc = 1.
            //both default arguments are applied.
        }
        else if(argc < 3){ //argc = 2.
            std::cerr << "Specify: port, maxclients.\n" << std::endl;
            return close_main(-1);
        }
        else if(argc < 4){ //argc = 3.
            prt = std::stoi(argv[1]);
            clients_limit = std::stoi(argv[2]);
        }
        else{ //3 < argc.
            std::cerr << "Too many arguments specified.\n" << std::endl;
            return close_main(-1);
        }

        t_clients_list clients(static_cast<size_t>(clients_limit));

        //Creating an InventoryManager instance:
        //-----
        std::vector<Store::Item> items_vec;
        items_vec.reserve(13); //Pre-allocates memory block to avoid reallocations. :).
        items_vec.emplace_back(1, "Camera");
        items_vec.emplace_back(2, "Tripod");
        items_vec.emplace_back(3, "Laptop");
        items_vec.emplace_back(4, "Projector");
        items_vec.emplace_back(5, "Microphone");
        items_vec.emplace_back(6, "Speaker");
        items_vec.emplace_back(7, "HDMI_Cable");
        items_vec.emplace_back(8, "Ethernet_Cable");
        items_vec.emplace_back(9, "Keyboard");
        items_vec.emplace_back(10, "Mouse");
        items_vec.emplace_back(11, "Monitor");
        items_vec.emplace_back(12, "USB_Hub");
        items_vec.emplace_back(13, "Power_Bank");
        
        Store::InventoryManager inventory(std::move(items_vec)); //initiating an InventoryManager object called "inventory" with the items_vec vector of Item-type objects.
        //-----

        
        //1: (socket assigning):
        //-----
        server_fd = socket(AF_INET, SOCK_STREAM, 0); //server-File-Descriptor(on linux OS).
        //domain: AF_INET(AF = address family) = Internet Domain (AF_INET): The standard for network sockets, 
        //using IP addresses and port numbers to communicate across different machines over protocols like TCP/IP(IPv4 in our case).
        //
        //SOCK_STREAM = a socket type that provides reliable, ordered, and error-checked data delivery -
        //through a connection-oriented byte stream, primarily using the TCP protocol.
        //
        //protocol: 0 = Default Protocol: Because you requested a stream socket over IPv4, the OS defaults to TCP (IPPROTO_TCP).

        /////////timer1.reset_timer(); //1
        
        //modified socket behavior to allow immediate reuse of the port(Gemini's implementation...):
        //-------
        int opt = 1;
        setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        //-------
        //-----

        /////////timer1.reset_timer(); //2

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

        /////////timer1.reset_timer(); //3

        //3: (binding to the socket):
        //-----
        try{
            //bind input structure:
            //bind(socket_descriptor, pointer_to_struct, size_of_struct)
            if(bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
                throw Bind_Exception("first listening-socket binding socket operation failure in main.", errno); 
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
                    throw Bind_Exception("second listening-socket binding socket operation failure in main.", errno); //if this part of the code throws an - 
                    // error or fails is should just rethrow the Bind_Exception and finally take the entire code down with an error return value(-1).
                }
            }
        }
        //-----

        /////////timer1.reset_timer(); //4

        //4. (listening to clients):
        //-----
        int backlog = 20;
        listen(server_fd, backlog); //backlog = limiting the in-progress connections queue size.
        logger.log("listening on port - " + std::to_string(prt) + "...");
        //-----

        /////////timer1.reset_timer(); //5

        //5. (accepting a connection): 
        //-----
        std::string temp_name = "";
        //Acceptting and handling a client's connection:
        while(true){
            try{ // -> here we try to catch both the accept + thread, and the handle_client exceptions.
                //Accepting a new client's connection:
                const int client_fd = accept(server_fd, nullptr, nullptr); //each new socket connection established removes that - 
                //"in-progress" object from the "backlog" queue initialized inside the "listen" operation, allowing new in-progress connections to enter that queue. 
                //int client_fd =
                //addr = A pointer to a sockaddr structure.             }both addr and addr_len are set to nullptr because they don't yet know the actual values -
                //addr_len = A pointer to the size of that structure.   }attributes to each new socket opened, instead they will be assigned after the ack - phase.
                //standard error checks:
                if(client_fd < 0){
                    if(errno == EINTR) continue; //EINTR = a "recoverable" error signifying a simple interrupted call.
                    throw Socket_Exception("accepting socket operation failure in main.", errno); //done to document in the logger.
                }
                else{ //a 3-way handshake has been established: 
                    temp_name = clients.add_client(client_fd);
                    if(temp_name == ""){ //means that the clients list is full...
                        send_all(client_fd, "ERR PROTOCOL server is full.\n", "main");
                        throw Socket_Exception("server is full message in main for: client_fd=" + std::to_string(client_fd) + ".", errno);
                        close(client_fd);
                        continue;  
                    } 
                }
                //Creating a new thread in order to handle each client as concurrent processes:
                std::thread(handle_client, std::ref(client_fd), std::ref(clients), temp_name, std::ref(inventory)).detach(); //(sending inventory by reference...).
                //*Note - this type of manual socket-opening technique we use here is called "Blocking-socket opening".
                //the reason it is discouraged(compared to non-Blocking alternatives like using select()/poll()/epoll()) is because - 
                //every operation we do on a single socket(read/write...) haults the entire thread it occupies, thus enabling only the creation of - 
                //a single socket on each thread, making it much slower and inefficient for robust programs with many clients to handle concurrently.
            } //[handle_client, std::ref(client_fd), std::ref(clients), std::ref(current_name), std::ref(inventory)],[const int client_fd, t_clients_list& clients, std::string& client_name, Store::InventoryManager& inventory]
            catch (const Socket_Exception& e){ //most likely recoverable:
                continue; 
            }
            catch (const Timeout_Exception& e){ //most likely unrecoverable:
                throw; //rethrows Timeout_Exception.
            }
        }
    }
    catch (const Bind_Exception& e){
        close(server_fd);
        return close_main(-1);
    }
    catch (const Socket_Exception& e){
        close(server_fd);
        return close_main(-1);
    }    
    catch (const Timeout_Exception& e){
        close(server_fd);
        return close_main(-1);
    }
    
    //6. ("healthy" closing the server socket):
    //-----
    return close_main(0);
    //-----
}
//-----