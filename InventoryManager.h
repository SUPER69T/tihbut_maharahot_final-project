#ifndef InventoryManager_H
#define InventoryManager_H

#include "Item.h"
#include <string>
#include <vector>
#include <mutex>
#include <condition_variable>
//#include <exception>;

class InventoryManager{
    private:
        std::vector<Item&> items;  
        std::mutex mtx;
        std::condition_variable cv; 

    public:
        InventoryManager(Item& items);
        std::string listItems();
        void borrowItem(int itemId, const std::string& username);
        void returnItem(int itemId, const std::string& username);
        void waitUntilAvailable(int itemId, const std::string& username);
        Item& findItemById(int itemId)
};
#endif
