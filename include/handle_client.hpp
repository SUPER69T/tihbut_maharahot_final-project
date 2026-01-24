#ifndef HANDLE_CLIENT_H
#define HANDLE_CLIENT_H

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string>
#include "InventoryManager.hpp"

//receiving a line:
bool recv_line(int fd, std::string& out);

//Sending an entire message:
void send_all(int fd, const std::string& msg);

//Checking whether a string is all digits:
bool is_number(const std::string& s);

//main client handling:
void handle_client(int client_fd, Store::InventoryManager& inventory);
#endif //HANDLE_CLIENT_H