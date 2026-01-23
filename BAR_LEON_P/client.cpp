#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>
#include <string>
#include <thread>
#include <vector>

// Read one line until '\n'
bool recv_line(int fd, std::string &out)
{
    out.clear();
    char c;
    while (true)
    {
        ssize_t n = recv(fd, &c, 1, 0);
        if (n <= 0)
            return false; // if the connection is closed
        if (c == '\n')
            break; // if we reach end of line
        out.push_back(c);
    }
    return true;
}

// Send the entire message makes sure all data is sent in the tcp protocol
void send_all(int fd, const std::string &msg)
{
    size_t sent = 0;
    while (sent < msg.size())
    {
        ssize_t n = send(fd, msg.data() + sent, msg.size() - sent, 0);
        if (n <= 0)
            return; // problem with sending
        sent += n;
    }
}
void listen_to_server(int fd){
    std::string reply;
    while (recv_line(fd, reply))
    {
        std::cout << "\n[Server]: " << reply << "\n> " << std::flush;
    }
}
void client_thread(std::string server_ip, int port)
{
    // Set up client socket
    int fd = socket(AF_INET, SOCK_STREAM, 0);

    if (fd < 0)
        return; // if there is an error creating the socket

    sockaddr_in srv{};
    srv.sin_family = AF_INET;
    // the port and address should match the server settings
    srv.sin_port = htons(port);
    // localhost address

    // .c_str() converts std::string to const char*
    inet_pton(AF_INET, server_ip.c_str(), &srv.sin_addr);

    // Handshake with the server
    if (connect(fd, (sockaddr *)&srv, sizeof(srv)) < 0)
    {
        std::cout << "connect error" << std::endl;
        close(fd); // close the socket if there is an error
        return;
    }
    std::thread listener(listen_to_server, fd);
    listener.detach();
    std::string st;

    // std::string reply;
    // if (recv_line(fd, reply)) {
    //     std::cout << reply << std::endl;
    // } DOESNT WORK

    while (true)
    {

        // Get user input(command)
        std::getline(std::cin, st);
        st += '\n'; // the rules of the project is to end each command with \n
        // Send command to server
        send_all(fd, st);

        if (st == "QUIT\n")
        {
            break; // exit the client if the command is exit
        }
    }
    close(fd); // close the socket
}
// argc: argument count,argv : argument vector for us argv[0] is the program name,argv[1] is the server IP, argv[2] is the server port 
int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        std::cerr << "Usage: client\n";
        return 1;
    }
   
    std::string server_ip = argv[1];
    int server_port = std::stoi(argv[2]);
    client_thread(server_ip,server_port);
    return 0;
}
