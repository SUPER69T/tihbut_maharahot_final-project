// tcp_server_simple.cpp
#include <arpa/inet.h>   // htons, htonl
#include <sys/socket.h>  // socket, bind, listen, accept, send, recv
#include <unistd.h>      // close
#include <cstring>       // strlen
#include <iostream>

int main() {
    // 1. יצירת socket TCP
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    // 2. הגדרת כתובת השרת
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;          // IPv4
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // כל כתובת מקומית
    server_addr.sin_port = htons(5555);        // פורט 5555

    // 3. bind – קישור ה-socket לפורט
    bind(server_fd, (sockaddr*)&server_addr, sizeof(server_addr));

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
