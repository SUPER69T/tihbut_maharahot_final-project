#include <handle_client.hpp>
#include <stdexcept>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string>
#include "InventoryManager.hpp"
#include <Network_Exception.hpp>


//a safe sending of an entire string:
void send_all(int fd, const std::string& msg){
    size_t sent = 0;
    while(sent < msg.size()){ //only entering if the entire string hasn't been sent yet.
        ssize_t n = send(fd, msg.data() + sent, msg.size() - sent, MSG_NOSIGNAL); //:
        //type ssize_t = a signed integer mainly used in read(), write(), recv() system calls, returns negatives to indicate errors.
        //ssize_t n - possible values and meaning:
        //(0 < n): the amount of bytes sent by send() in the current while-loop iteration.
        //
        //(n = 0): the connection was closed normally(the receiver sent the graceful shutdown signal[FIN{FIN is a special flag} = 1]).
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
        
        if(n == 0){ //the entire file has been sent (no errors occured).
        close(fd); //cleaning up our side of the connection.
        return;
        }
        else if(n <= 0){// = if an error is thrown / the connection is closed...
            if(n != 11 | n != 35){
                throw Socket_Exception("send_all()", errno);
            }
        }
        else sent += n; //continuing the iteration - there are more bytes to be transferred. 
    }
}

//TCP framing of each byte at a time, with a max of: default=1024 bytes, until we reach - '\n':
bool recv_line(int fd, std::string& out, size_t max_len = 4096){
    out.clear();
    char c;
    while(out.size() < max_len) {
        ssize_t n = recv(fd, &c, 1, 0); //:
        //this approach is highly inefficient due to repeatd calls to recv() for every single input character.
        //a better approach would involve buffering larger chunks of bytes each time and keeping track of the -
        //current pos-index location on the buffered input stream. 

        if(n == 0) return false; //the connection was closed normally.
        if(n < 0){ //best practice would adding additional checked conditions inside try-catch blocks for debugging.
            return false; 
        }
        if(c == '\n') return true; //finished receiving the entire message.
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

void handle_client(int client_fd, Store::InventoryManager& inventory){

    bool is_authenticated = false; //checking whether the user sent an entry - "Hello" message. 
    std::string username; 

    //Process-client commands:
    while(true){
        bool check_username = true; //to authenticating the username.

        std::string line;

        if(!recv_line(client_fd, line, 4096)) break; //the client disconnected 

        // Split command and argument we need how to message will come
        std::string command;
        std::string arg;
        size_t space_pos=line.find(' '); //find space to split command and argument

        if(space_pos!=std::string::npos){ // if there is space we take the first half to be the commend and the second to be the argument

            command=line.substr(0,space_pos);
            arg=line.substr(space_pos+1);
        }

        else{
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
            for(char c : username){
                if(!isalpha(c)){

                    is_authenticated = false;

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
            //catch if item is not available
            catch(const std::runtime_error& e){
                send_all(client_fd, e.what());
            }
        }

        // Return an item
        else if(command=="RETURN"){

            int itemID;
            try{
                //Convert argument to integer
                itemID=std::stoi(arg);

                inventory.returnItem(itemID,username);

                send_all(client_fd, "OK RETURN "+std::to_string(itemID)+"\n");
            }

            //if the argument is not a valid integer
            catch(const std::invalid_argument&){
                send_all(client_fd, "ERR PROTOCOL invalid_id\n");
            }

            //if there is an error during return not the borrower or item not found
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
        else if(command == "QUIT"){
            send_all(client_fd, "OK QUIT\n");
            break;
        }
    } 

    // Close client connection
    close(client_fd);
}