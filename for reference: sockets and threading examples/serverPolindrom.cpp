#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

// קריאה של שורה אחת עד '\n' (framing פשוט ל-TCP)
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

// שליחה בטוחה של כל המחרוזת
void send_all(int fd, const std::string& msg) {
    size_t sent = 0;
    while (sent < msg.size()) {
        ssize_t n = send(fd, msg.data() + sent, msg.size() - sent, 0);
        if (n <= 0) return;
        sent += n;
    }
}

// בדיקת פולינדרום לפי סדר מילים
bool is_word_palindrome(const std::string& sentence) {
    std::istringstream iss(sentence);
    std::vector<std::string> words;
    std::string w;

    while (iss >> w) {
        words.push_back(w);
    }

    for (size_t i = 0, j = words.size() - 1; i < j; i++, j--) {
        if (words[i] != words[j]) {
            return false;
        }
    }
    return true;
}

// טיפול בלקוח בודד (רץ בתהליכון נפרד)
void handle_client(int client_fd) {
    std::string line;

    // מצפים לפורמט: id|sentence\n
    if (!recv_line(client_fd, line)) {
        close(client_fd);
        return;
    }

    size_t sep = line.find('|');
    if (sep == std::string::npos) {
        send_all(client_fd, "Bad format\n");
        close(client_fd);
        return;
    }

    int id = std::stoi(line.substr(0, sep));
    std::string sentence = line.substr(sep + 1);

    bool pal = is_word_palindrome(sentence);

    std::string reply;
    if (pal) {
        reply = "Thread " + std::to_string(id) +
                " sent a WORD palindrome\n";
    } else {
        reply = "Thread " + std::to_string(id) +
                " sent NOT a WORD palindrome\n";
    }

    send_all(client_fd, reply);
    close(client_fd);
}

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(5555);

    bind(server_fd, (sockaddr*)&addr, sizeof(addr));
    listen(server_fd, 10);

    std::cout << "Server listening on port 5555\n";

    while (true) {
        int client_fd = accept(server_fd, nullptr, nullptr);
        std::thread(handle_client, client_fd).detach();
    }
}
