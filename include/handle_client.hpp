#ifndef HANDLE_CLIENT_H
#define HANDLE_CLIENT_H

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string>
#include "InventoryManager.hpp"
#include <t_clients_list.hpp>

//just for fun...:
void close_client_thread(const int client_fd, const std::string confirmed_name, t_clients_list& clients_list);

//sending an entire message:
void send_all(int fd, const std::string& confirmed_name, t_clients_list& clients_list, const std::string& msg);

//receiving a line:
bool recv_line(const int fd, const std::string& confirmed_name, t_clients_list& clients_list, std::string& out, size_t max_len);

//checking whether a string is all digits:
bool is_number(const std::string& s);

//main client handling:
void handle_client(const int client_fd, t_clients_list& clients_list, std::string temp_name, Store::InventoryManager& inventory);

#endif //HANDLE_CLIENT_H