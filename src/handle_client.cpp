#include <stdexcept>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string>
#include <map>
#include <cstdlib>
#include "handle_client.hpp"
#include "InventoryManager.hpp"
#include "Network_Exception.hpp"
#include "Item_exception.hpp"
#include "IM_exception.hpp"
#include "t_clients_list.hpp"


//a safe sending of an entire string:
void send_all(int fd, const std::string& msg, const std::string& confirmed_name){
    size_t sent = 0;
    while(sent < msg.size()){ //only entering if the entire string hasn't been sent yet.
        ssize_t n = send(fd, msg.data() + sent, msg.size() - sent, MSG_NOSIGNAL); //:
        //type ssize_t = a signed integer mainly used in read(), write(), recv() system calls, returns negatives to indicate errors.
        //ssize_t n - possible values and meaning:
        //(0 < n): the amount of bytes sent by send() in the current while-loop iteration.
        //
        //(n = 0): the connection was closed gracefully(the receiver sent the graceful shutdown signal[FIN{FIN is a special flag} = 1]).
        //
        //(n = -1): an error has been thrown. possible errors:
        //__________________________________________________________________________________________________________________________|
        //errno code:          | cause of error:                                                                                    |
        //_____________________|____________________________________________________________________________________________________|
        //EAGAIN / EWOULDBLOCK - Socket buffer is full(a good idea can be to wait for the buffer to flush and be reusable again...).|
        //EBADF	               - Invalid file descriptor: The fd is closed or was never opened.                                     |
        //ECONNRESET           - Client crashed or hard-closed. (should close our end of the socket for security reasons...).       |
        //EPIPE	Broken pipe    - tried to send to a socket that is already closed.                                                  |
        //EINTR	Interrupted    - A signal interrupted the call before data was sent.                                                |
        //EMSGSIZE             - The message exceeds the max size supported by the protocol.                                        |
        //__________________________________________________________________________________________________________________________|
        //
        //sent += n: this n value is then used to count the total amount of bytes sent.
        //
        ////<string>.data():  points at the first char of the char-array which is the string - allows access to the internal data buffer managed by the std::string object.
        //
        //buf = msg.data() + sent: start index from which send() will start sending bytes(the offset from the first char).
        //n = msg.size() - sent: acts as a safety boundary so the kernel doesn't read past the end of the string's memory.
        //flags:
        //0: Default behavior (identical to write()).
        //MSG_NOSIGNAL: Prevents the program from crashing if the client abruptly disconnects (prevents a - SIGPIPE signal).
        //MSG_DONTWAIT: Makes the call non-blocking for just this one specific operation.
        
        if(n < 0){ //if an error is thrown:
            //checking for recoverable errors:
            if(errno == EAGAIN || errno == EWOULDBLOCK){ //errno = 11: EAGAIN(in linux), errno: 35 = EAGAIN(in macOS/BSD).
                continue;
            }
            throw Socket_Exception("ERR PROTOCOL" + confirmed_name + " had trouble in sending the message.", errno);
        }
        ///*
        else if(n == 0){ //0 bytes have been transferred, which isn't usual socket behavior...
            throw Socket_Exception("ERR PROTOCOL" + confirmed_name +" sent 0 bytes unexpectedly.", errno);
        }
        //*/
        sent += n; //continuing the iteration - there are more bytes to be transferred. 
    }
}

//TCP framing of each byte at a time, with a max of: default=1024 bytes, until we reach - '\n':
bool recv_line(const int fd, std::string& out, size_t max_len = 4096){
    out.clear();
    char c;
    while(out.size() < max_len) {
        ssize_t n = recv(fd, &c, 1, 0); //:
        //this approach is highly inefficient due to repeated calls to recv() for every single input character.
        //a better approach would involve buffering larger chunks of bytes each time and keeping track of the -
        //current pos-index location on the buffered input stream. 

        if(n == 0) return false; //the connection was closed gracefully.
        if(n < 0){ //best practice would adding additional checked conditions inside try-catch blocks for debugging.
            return false; 
        }
        if(c == '\n') return true; //finished receiving the entire message.
        out += c; //continuing the iteration - there are more bytes to be received.
    }
    return false; //Reaching max_len without finding newline.
}

//Checking whether a string is all digits: 
bool is_number(const std::string& s){
    for(char c : s){
        if(!isdigit(c)) return false;
    }
    return true;
}

void handle_client(const int client_fd, t_clients_list& clients, std::string& temp_name, Store::InventoryManager& inventory){

    bool is_authenticated = false; //checking whether the user sent an entry - "Hello" message. 

    //variable initializations for the while loop:
    //---
    bool check_username = true; //to authenticating the username.
    std::string confirmed_name = temp_name; //unconfirmed yet...
    std::string line;
    std::string command;
    std::string arg; //used for general-purpose input checking and de-muxing logic.
    int itemID;
    int rand_int;
    bool exit_flag = false;

    //a cute alternative to switch-case:
        enum class Command {LIST, BORROW, RETURN, WAIT, QUIT};

        //a python-style helper map:
        std::map<std::string, Command> commandMap = {
            {"LIST", Command::LIST},
            {"BORROW", Command::BORROW},
            {"RETURN", Command::RETURN},
            {"WAIT", Command::WAIT},
            {"QUIT", Command::QUIT}
        };

        std::vector<std::string> defaultMessages = {
    "SORRY, I DID NOT GET THAT, CAN YOU PLEASE REPEAT?",
    "UNKNOWN COMMAND. TRY FOLLOWING INSTRUCTION, MAYBE.",
    "HUH? THAT'S NOT AN OPTION, TRY AGAIN!"
    };
    //---

    //process client-commands:         
    while(true){ //handle_client's terminal-like - loop:
        try{
            if(!recv_line(client_fd, line, 4096)){ //a check that closes the socket if the client disconnected or on other various 'recv' errors. 
                throw Socket_Exception("ERR PROTOCOL " + confirmed_name + " disconnected from server.", errno);
            } 
            size_t space_pos = line.find(' '); //finds 'space' and splits into: command and argument.

            if(space_pos != std::string::npos){ //if there is a space we take the first half to be the commend and the second half to be the argument.
                command = line.substr(0,space_pos);
                arg = line.substr(space_pos+1);
            }
            else{//if there is no space the whole line is the command(only - LIST or QUIT).
                command = line;
                arg = ""; //resetting arg.
            } 
            itemID = -1; //reseting default itemID to -1.
            if(!is_authenticated){ //in case the name-authentication hasn't occurred yet:
                if(command != "HELLO"){ //in case the client hasn't commanded "HELLO" now:
                    throw std::invalid_argument("ERR PROTOCOL you have to authenticate yourself first.");
                    continue; 
                }
                else if(command == "HELLO"){ //trying to authenticate now:
                    check_username = true; //reseting default check_username flag position.

                    //checks if the username is missing:
                    if(arg.empty()){ 
                        throw std::invalid_argument("ERR PROTOCOL missing_username."); 
                        check_username = false;
                        continue; 
                    }
                    //making sure that the user only contains letters:
                    for(char c : arg){
                        if(!isalpha(c)){
                            is_authenticated = false;
                            check_username = false;
                            throw std::invalid_argument("ERR PROTOCOL invalid_username.");
                        }
                    }
                    //if the username is valid:
                    if(check_username){
                        try{
                            if(!clients.add_client(client_fd, arg)){ //a one-time updating of the client_name in the threaded clients-list.
                                throw Socket_Exception("SYSTEM ERR PROTOCOL " + confirmed_name + "'s name got corrupted.", errno);
                            } 
                        }
                        catch(const Socket_Exception& e){ //a hard-stop program-shutdown in the case of name-corruption.
                            send_all(client_fd, e.what(), confirmed_name); //:
                            //note on the send_all design: this is an ugly alternative to try-catching every single send_all function call, but - 
                            //that would also be ugly...cpp doesn't make exception handling/throwing/rethrowing easy so even an ugly solution can be viable. 
                            exit(1);
                        }
                        //in case all went well with appending to the clients list:
                        //
                        is_authenticated = true; //the user is valid.
                        confirmed_name = arg;
                        send_all(client_fd, "OK HELLO, " + confirmed_name +"!\n", confirmed_name);
                        //
                        //authentication completed.
                    }
                }  
            }
            //just for fun:
            if(command == "HELLO" and is_authenticated){
                throw std::invalid_argument("you know you don't have to keep authenticating...right?");
            }
            //
//____________________________________________________________________________________________________
//now that the user is authenticated we can continue into the switch-case:
//____________________________________________________________________________________________________
            //converting the argument into an integer for BORROW/RETURN/WAIT:
            if(arg != ""){ //this is true only in the cases where the client didn't choose LIST/QUIT.
                try{
                    itemID = std::stoi(arg); //used in the switch-case statement for Item/InventoryManager classes.
                }
                catch(std::invalid_argument& e){
                    throw std::invalid_argument("received an invalid ID.");
                }
            }
//moving forward into the switch statment only if is_authenticated is 'true':
//____________________________________________________________________________________________________
//there in no switch-case for strings in cpp? really??
//____________________________________________________________________________________________________
            //acts as a default condition inside the switch-case statement:
            //---
            auto it = commandMap.find(command);
            if(it == commandMap.end()){ //iterator ran over the entire map and didn't find "command" as a key.
                rand_int = (rand() % 3); //a random value between - 0, and - 2.
                send_all(client_fd, defaultMessages.at(rand_int), confirmed_name);
            }
            //---

            //-----
            //checking whether the client has sent an itemID:
            Command cmd = commandMap[command];
            if((cmd == Command::BORROW || cmd == Command::RETURN || cmd == Command::WAIT) && itemID == -1){ //in cases where an itemID hasn't been given at all.
                throw std::invalid_argument("ID not specified.");
            }

            switch(cmd){

                //---
                //shows the list of items: 
                case Command::LIST:{
                    //gets the list of items from inventory:
                    std::string response = inventory.listItems();
                    //sends the response to the client:
                    send_all(client_fd,response+"\n", confirmed_name);
                    break;
                }
                //---

                //---
                //borrow an item:
                case Command::BORROW:{
                    //attempts to borrow the item from the InventoryManager:
                    inventory.borrowItem(itemID, confirmed_name);
                    send_all(client_fd, "OK BORROW "+std::to_string(itemID) + "\n", confirmed_name);
                    break;
                }
                //---

                //---
                //returns an item:
                case Command::RETURN:{
                    inventory.returnItem(itemID, confirmed_name);
                    send_all(client_fd, "OK RETURN " + std::to_string(itemID) + "\n", confirmed_name);
                    break;
                }
                //---

                //---
                //waits until an item is available:
                case Command::WAIT:{
                    //the thread will wait until the item is available. it isn't practical for real life code implementation but for this project it will have to do...:
                    inventory.waitUntilAvailable(itemID, confirmed_name);
                    send_all(client_fd, "OK AVAILABLE " + std::to_string(itemID) + "\n", confirmed_name);
                    break;
                }
                //---

                //---
                //stops the connection between the server and the client:
                case Command::QUIT:{
                    send_all(client_fd, "OK QUIT\n", confirmed_name);
                    exit_flag = true;
                    break;
                }
                //---
            }
            //-----
        }
        catch(const Store::Item_exception& e){ //acts as a soft-exception in cases related to Item-exceptions.
            send_all(client_fd, std::string("ERR ") + e.what() + "\n", confirmed_name);
            continue;
        }
        catch(const Store::IM_exception& e){ //acts as a soft-exception in cases related to InventoryManager-exceptions.
            send_all(client_fd, std::string("ERR ") + e.what() + "\n", confirmed_name);
            continue;
        }
        catch(const Socket_Exception& e){ //acts as a socket-closer in cases of socket-exceptions.
            break;
        }
        catch(const Timeout_Exception& e){ //acts as a socket-closer in cases of timeout-exceptions.
            send_all(client_fd, e.what() + std::string("\n"), confirmed_name);
            break;
        }
        catch(const std::invalid_argument& e){ //acts as a soft-exception in cases related to invalid-arguments.
            send_all(client_fd, std::string("ERR STATE for - ") + confirmed_name + ": " + e.what() + "\n", confirmed_name);
            continue;
        }

        if(exit_flag == true) break;
    } 

    //closing the connection:
    if(clients.remove_client(client_fd)) close(client_fd);//removing the client from the threaded-clients list.
    exit(1); //killing the whole program as a safety measure(file corruption with the clients list...).
}