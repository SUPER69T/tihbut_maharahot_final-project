// tcp client establishing:
#include <arpa/inet.h>   // inet_pton
#include <sys/socket.h>  // socket, connect, send, recv
#include <unistd.h>      // close
#include <cstring>       // strlen
#include <iostream>

int main() {
    // 1. יצירת socket TCP
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);

    // 2. הגדרת כתובת השרת
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;          // IPv4
    server_addr.sin_port = htons(8080);        // פורט השרת
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr); // localhost

    // 3. התחברות לשרת
    connect(client_fd, (sockaddr*)&server_addr, sizeof(server_addr));

    // 4. שליחת הודעה לשרת
    const char* msg = "Hello from client";
    send(client_fd, msg, strlen(msg) + 1, 0);

    // 5. קבלת תשובה מהשרת
    char buffer[1024];
    recv(client_fd, buffer, sizeof(buffer), 0);

    std::cout << "Received from server: " << buffer << std::endl;

    // 6. סגירת socket
    close(client_fd);

    return 0;
}
