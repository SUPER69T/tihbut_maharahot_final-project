// EXAMPLE 1:

// #include <iostream>
// #include <thread>
// #include <mutex>
// #include <condition_variable>
// #include <chrono>

// std::mutex mtx;
// std::condition_variable cv;
// bool ready = false;

// void waiter(int id) {
//     std::unique_lock<std::mutex> lock(mtx);  
//     while (!ready) {
//         cv.wait(lock);                      
//     }
//     std::cout << "[waiter " << id << "] continuing\n";
// }

// void notifier() {
//     std::this_thread::sleep_for(std::chrono::seconds(2));

//     mtx.lock();
//     ready = true;
//     mtx.unlock();

//     cv.notify_all();
// }

// int main() {
//     std::thread t1(waiter, 1);
//     std::thread t2(waiter, 2);
//     std::thread t3(notifier);

//     t1.join();
//     t2.join();
//     t3.join();
// }

// EXAMPLE 2:

// #include <mutex>
// #include <condition_variable>

// std::mutex mtx;
// std::condition_variable cv;
// bool ready = false;

// void waiter() {
//     std::unique_lock<std::mutex> lock(mtx); // Lock the mutex

//     while (!ready) {
//         cv.wait(lock); // Releases the mutex while waiting,
//                        // re-locks it after waking up
//     }

//     // At this point: ready == true and the mutex is locked
// }

// void notifier() {
//     mtx.lock();        // Lock mutex before changing shared state
//     ready = true;
//     mtx.unlock();      // Unlock mutex

//     cv.notify_all();   // Wake up all waiting threads
// }

// EXAMPLE 3:

// #include <mutex>
// #include <condition_variable>

// using namespace std;

// class semaphore {
// public:
//     semaphore(int value = 0) : value(value) {}

//     void wait() {
//         unique_lock<mutex> lock(m);
//         while (value == 0) {
//             cv.wait(lock);
//         }
//         --value;
//     }

//     void signal() {
//         unique_lock<mutex> lock(m);
//         ++value;
//         cv.notify_one();
//     }

// private:
//     int value;
//     mutex m;
//     condition_variable_any cv;
// };


// EXAMPLE 4:

// #include <iostream>
// #include <thread>
// #include <vector>

// using namespace std;

// semaphore sem(2);

// void worker(int id) {
//     cout << "Thread " << id << " waiting for a resource\n";
//     sem.wait();

//     cout << "Thread " << id << " using a resource\n";

//     cout << "Thread " << id << " releasing resource\n";
//     sem.signal();
// }

// int main() {
//     vector<thread> threads;

//     for (int i = 1; i <= 5; ++i) {
//         threads.emplace_back(worker, i);
//     }

//     for (auto& t : threads) {
//         t.join();
//     }

//     return 0;
// }

// EXAMPLE 5:

// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <unistd.h>

// int main() {
//     int server = socket(AF_INET, SOCK_STREAM, 0);

//     sockaddr_in addr{};
//     addr.sin_family = AF_INET;
//     addr.sin_addr.s_addr = INADDR_ANY;
//     addr.sin_port = htons(1234);

//     bind(server, (sockaddr*)&addr, sizeof(addr));
//     listen(server, 1);

//     int client = accept(server, nullptr, nullptr);

//     write(client, "Hi", 2);

//     close(client);
//     close(server);
// }

// EXAMPLE 6:

// #include <sys/socket.h>
// #include <arpa/inet.h>
// #include <unistd.h>

// int main() {
//     int sock = socket(AF_INET, SOCK_STREAM, 0);

//     sockaddr_in addr{};
//     addr.sin_family = AF_INET;
//     addr.sin_port = htons(1234);
//     inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

//     connect(sock, (sockaddr*)&addr, sizeof(addr));

//     char buf[3] = {};
//     read(sock, buf, 2);

//     close(sock);
// }

// EXAMPLE 7:

// #include <arpa/inet.h>
// #include <sys/socket.h>
// #include <unistd.h>

// #include <iostream>
// #include <string>
// #include <thread>
// #include <vector>

// using namespace std;

// bool recv_line(int fd, string& out) {
//     out.clear();
//     char c;
//     while (true) {
//         ssize_t n = recv(fd, &c, 1, 0);
//         if (n <= 0) return false;
//         if (c == '\n') break;
//         out.push_back(c);
//     }
//     return true;
// }

// void send_all(int fd, const string& msg) {
//     size_t sent = 0;
//     while (sent < msg.size()) {
//         ssize_t n = send(fd, msg.data() + sent, msg.size() - sent, 0);
//         if (n <= 0) return;
//         sent += (size_t)n;
//     }
// }

// void handle_client(int client_fd) {
//     string line;
//     if (!recv_line(client_fd, line)) { close(client_fd); return; }

//     send_all(client_fd, "Server got: " + line + "\n");
//     close(client_fd);
// }

// int main() {
//     const int N = 3;  

//     int server_fd = socket(AF_INET, SOCK_STREAM, 0);

//     sockaddr_in addr{};
//     addr.sin_family = AF_INET;
//     addr.sin_addr.s_addr = htonl(INADDR_ANY);
//     addr.sin_port = htons(5555);

//     bind(server_fd, (sockaddr*)&addr, sizeof(addr));
//     listen(server_fd, 10);

//     cout << "Server listening on 5555\n";

//     vector<thread> workers;
//     workers.reserve(N);

//     for (int i = 0; i < N; ++i) {
//         int client_fd = accept(server_fd, nullptr, nullptr);
//         workers.emplace_back(handle_client, client_fd);
//     }

//     for (thread& t : workers) {
//         t.join();
//     }

//     close(server_fd);
// }

// EXAMPLE 8:

// #include <arpa/inet.h>
// #include <sys/socket.h>
// #include <unistd.h>

// #include <iostream>
// #include <string>
// #include <thread>
// #include <vector>

// using namespace std;

// bool recv_line(int fd, string& out) {
//     out.clear();
//     char c;
//     while (true) {
//         ssize_t n = recv(fd, &c, 1, 0);
//         if (n <= 0) return false;
//         if (c == '\n') break;
//         out.push_back(c);
//     }
//     return true;
// }

// void send_all(int fd, const string& msg) {
//     size_t sent = 0;
//     while (sent < msg.size()) {
//         ssize_t n = send(fd, msg.data() + sent, msg.size() - sent, 0);
//         if (n <= 0) return;
//         sent += (size_t)n;
//     }
// }

// void client_job(int id) {
//     int fd = socket(AF_INET, SOCK_STREAM, 0);

//     sockaddr_in srv{};
//     srv.sin_family = AF_INET;
//     srv.sin_port = htons(5555);
//     inet_pton(AF_INET, "127.0.0.1", &srv.sin_addr);

//     connect(fd, (sockaddr*)&srv, sizeof(srv));

//     send_all(fd, to_string(id) + "\n");

//     string reply;
//     if (recv_line(fd, reply)) {
//         cout << reply << endl;
//     }

//     close(fd);
// }

// int main() {
//     const int N = 3;

//     vector<thread> threads;
//     for (int i = 0; i < N; ++i) {
//         threads.emplace_back(client_job, i);
//     }
//     for (thread& t : threads) {
//         t.join();
//     }
// }
