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
        // if client disconnects
        if (n <= 0) return false;
        // if we reach end of line
        if (c == '\n') break;
        // append character to output
        out.push_back(c);
    }
    return true;
}

// Send the entire message
// the loop make sure tat all data is sent in the tcp protocol
void send_all(int fd, const std::string& msg) {
    size_t sent = 0;
    while (sent < msg.size()) {
        ssize_t n = send(fd, msg.data() + sent, msg.size() - sent, 0);
        if (n <= 0) return;
        sent += n;
    }

}
// Check if a string is a number 
bool is_number(const std::string& s) {
    for (char c : s) {
        if (!isdigit(c)) return false;
    }
    return true;
}
// Handle client connection
void handle_client(int client_fd, InventoryManager& inventory) {
    bool is_authenticated=false; //check if the user did Hello 
    std::string username;   //UserName Name
    // Process client commands
    while(true){
        bool check_username=true; //to valid the username (while we run we can see if we realy need another check if the user is authenticated)

        std::string line;

        if(!recv_line(client_fd, line)) break; //the client disconnected 
        // Split command and argument we need how to message will come
        std::string command;
        std::string arg;
        size_t space_pos=line.find(' '); //find space to split command and argument

        if(space_pos!=std::string::npos){ // if there is space we take the first half to be the commend and the second to be the argument
            command=line.substr(0,space_pos);
            arg=line.substr(space_pos+1);

        }else{
            command=line; //if the is no space the whole line is the command(like LIST or QUIT)
        }

        if (command=="HELLO"){ //first command to authenticate the user
                username=arg; 
                if(username.empty()){ //check if the username is missing
                    send_all(client_fd, "ERR PROTOCOL missing_username\n");
                    check_username=false;
                    continue; 
                }
                //check if the username contains only letters
                for (char c : username){
                    if(!isalpha(c)){
                        is_authenticated=false;
                        send_all(client_fd, "ERR PROTOCOL invalid_username\n");
                        check_username=false;
                        break;
                       
                    }
                }
                // If username is valid
                if(check_username){
                    is_authenticated=true; //the user is valid
                    send_all(client_fd, "OK HELLO\n");

                }
                
                    
                }  
                // If the user is not authorized helps us to avoid to "skip" the HELLO command
                else if(!is_authenticated){
                    send_all(client_fd, "ERR STATE not_authenticated\n");
                    continue;

            }

                //Show the list of items            
                else if(command=="LIST"){
                    // Get the list of items from inventory
                    std::string response=inventory.listItems();
                    // Send the response to the client
                    send_all(client_fd,response+"\n");
            
                }
                // Borrow an item
                else if(command=="BORROW"){
                    int itemID;
                    try{
                        // Convert argument to integer
                        itemID=std::stoi(arg);
                        // Attempt to borrow the item going to InventoryManager to check if is available if it is we use the borrow method in Item
                        inventory.borrowItem(itemID,username);
                        send_all(client_fd, "OK BORROW "+std::to_string(itemID)+"\n");
                    }
                    catch(const std::invalid_argument&){
                        // if the argument is not a valid integer
                        send_all(client_fd, "ERR PROTOCOL invalid_id\n");
                    }
                //  catch if item is not available
                    catch(const std::runtime_error& e){
                        send_all(client_fd, e.what());
                    }
                        
                    
                    
                }
                // Return an item
                else if(command=="RETURN"){
                    int itemID;
                    try{
                        // Convert argument to integer
                        itemID=std::stoi(arg);
                        inventory.returnItem(itemID,username);
                        send_all(client_fd, "OK RETURN "+std::to_string(itemID)+"\n");
                    }
                    // if the argument is not a valid integer
                    catch(const std::invalid_argument&){
                        send_all(client_fd, "ERR PROTOCOL invalid_id\n");
                    }
                    //  if there is an error during return not the borrower or item not found
                    catch(const std::runtime_error& e){
                        send_all(client_fd, e.what());
                    }
                }
                // Wait until an item is available
                else if(command=="WAIT"){
                    int itemID;
                    try{
                        itemID=std::stoi(arg);
                        // the thread will wait until the item is available its not good in the real life scenario but for this project is ok(can make a long queue )
                        inventory.waitUntilAvailable(itemID,username);
                        send_all(client_fd, "OK AVAILABLE "+std::to_string(itemID)+"\n");
                    }
                    catch(const std::invalid_argument&){
                        send_all(client_fd, "ERR PROTOCOL invalid_id\n");
                    }
                    catch(const std::runtime_error& e){
                        send_all(client_fd, e.what());
                    }
                }
                // Stops the connection between server and client
                else if(command=="QUIT"){
                    send_all(client_fd, "OK QUIT\n");
                    break;
                }
                
            } 
            // Close client connection
            close(client_fd);
            }
            
       

        

// argc: argument count,argv : argument vector for us argv[0] is the program name, argv[1] is the port number
int main(int argc, char *argv[]){
    if(argc!=2){
        std::cerr<<"Usage: server port\n";
        return 1;
    }
    int port=std::stoi(argv[1]);
    // Create InventoryManager instance
    InventoryManager items;
    // Set up server socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0); //server-File-Descriptor(on linux OS).
    //domain: AF_INET(AF = address family) = Internet Domain (AF_INET): The standard for network sockets, 
    //using IP addresses and port numbers to communicate across different machines over protocols like TCP/IP(IPv4 in our case).
    //
    //SOCK_STREAM = a socket type that provides reliable, ordered, and error-checked data delivery -
    //through a connection-oriented byte stream, primarily using the TCP protocol.
    //
    //protocol: 0 = Default Protocol: Because you requested a stream socket over IPv4, the OS defaults to TCP (IPPROTO_TCP).

    // Set socket options to reuse address and port
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    //bind input structure:
    //bind(socket_descriptor, pointer_to_struct, size_of_struct)
    bind(server_fd, (sockaddr*)&addr, sizeof(addr));
         
    
    
    listen(server_fd, 10);

    std::cout << "Server listening on port" <<port << std::endl;
    // Accept and handle client connections
    while (true) {

        // Accept a new client connection
        int client_fd = accept(server_fd, nullptr, nullptr);

        // Create a new thread to handle the client
        std::thread(handle_client, client_fd, std::ref(items)).detach();
    }
    // close The server socket

    close(server_fd);
    return 0;
    
    
} ;  
