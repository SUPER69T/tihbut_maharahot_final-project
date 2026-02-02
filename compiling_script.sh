#!/bin/bash

mkdir -p bin

cd src

echo "Compiling Server..."
g++ -I../include server.cpp Item.cpp InventoryManager.cpp handle_client.cpp t_clients_list.cpp thread_safe_logger.cpp threaded_t_timer.cpp -o ../bin/server -pthread

echo "Compiling Client..."
g++ -I../include client.cpp -o ../bin/client -pthread

cd ..
