// tcp_server_threaded.cpp
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <thread>

void handle_client(int client_fd) {
    // פונקציה שרצה בתהליכון נפרד עבור כל לקוח

    char buffer[1024];

    // קבלת הודעה אחת מהלקוח
    recv(client_fd, buffer, sizeof(buffer), 0);

    std::cout << "Client says: " << buffer << std::endl;

    // שליחת תשובה
    const char* reply = "Hello from threaded server";
    send(client_fd, reply, strlen(reply) + 1, 0);

    // סגירת החיבור עם הלקוח
    close(client_fd);
}

int main() {
    // 1. יצירת socket TCP
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    // 2. הגדרת כתובת השרת
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(5555);

    // 3. bind
    bind(server_fd, (sockaddr*)&server_addr, sizeof(server_addr));

    // 4. listen
    listen(server_fd, 5);

    std::cout << "Threaded server listening on port 5555\n";

    // 5. לולאת accept
    while (true) {
        int client_fd = accept(server_fd, nullptr, nullptr);

        // 6. יצירת תהליכון חדש לכל לקוח
        std::thread t(handle_client, client_fd);

        // 7. ניתוק התהליכון מה-main
        t.detach();
    }

    close(server_fd);
    return 0;
}
