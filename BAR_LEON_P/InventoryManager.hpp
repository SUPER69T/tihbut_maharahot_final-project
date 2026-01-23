#ifndef INVENTORYMANAGER_H
#define INVENTORYMANAGER_H

#include "Item.hpp"
#include <string>
#include <vector>
#include <mutex>
#include <condition_variable>

class InventoryManager{
    private:
        std::vector<Item> items;  
        std::mutex mtx;
        std::condition_variable cv; 
        std::string listItems(std::vector<Item>& items); //could have also implemented "string_view_literals".

        Item& findItemById(int itemId);

    public:
        //Constructors:
        InventoryManager(); //empty constructor.
        InventoryManager(const std::vector<Item>& items); //Item-type vector-reference receiving constructor.
        //
        std::string listItems();
        void borrowItem(int itemId, const std::string& username);
        void returnItem(int itemId, const std::string& username);
        void waitUntilAvailable(int itemId, const std::string& username);
        
};

#endif
