#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>
#include <string>
#include <thread>
#include <vector>

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

void send_all(int fd, const std::string& msg) {
    size_t sent = 0;
    while (sent < msg.size()) {
        ssize_t n = send(fd, msg.data() + sent, msg.size() - sent, 0);
        if (n <= 0) return;
        sent += n;
    }
}

void client_thread(int id, const std::string& sentence) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in srv{};
    srv.sin_family = AF_INET;
    srv.sin_port = htons(5555);
    inet_pton(AF_INET, "127.0.0.1", &srv.sin_addr);

    connect(fd, (sockaddr*)&srv, sizeof(srv));

    std::string msg = std::to_string(id) + "|" + sentence + "\n";
    send_all(fd, msg);

    std::string reply;
    recv_line(fd, reply);
    std::cout << reply << std::endl;

    close(fd);
}

int main() {
    std::vector<std::string> sentences = {
        "one two three two one",
        "this is not palindrome",
        "hello world hello"
    };

    std::vector<std::thread> threads;

    for (int i = 0; i < (int)sentences.size(); i++) {
        threads.emplace_back(client_thread, i, sentences[i]);
    }

    for (auto& t : threads) {
        t.join();
    }
}
