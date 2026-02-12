#ifndef INVENTORYMANAGER_H
#define INVENTORYMANAGER_H

#include "Item.hpp"
#include <string>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <atomic>

namespace Store{

    class InventoryManager{

        private:
        //-----
        //Fields:
        std::vector<Item> items;  
        std::mutex mtx;
        std::condition_variable cv; 
        std::string listItems(std::vector<Item>& items); //could have also implemented "string_view_literals".
        static std::atomic<int> total_IMs;
        int IM_Id = 0;

        //Private methods:
        Item& findItemById(const std::string& username, const int itemId);
        //
        //-----
        
        public:
        //-----
        //Constructors:
        InventoryManager(); //empty constructor.
        InventoryManager(std::vector<Item> items); //Item-type-vector copy-constructor.
        //

        //Destructor:
        ~InventoryManager() = default; //thought of implementing a manual destructor, but that -
        //seems unnecessary due to the defualt cpp RAII implementations of our used objects here...
        //

        //Public methods:
        std::string listItems();
        void borrowItem(int itemId, const std::string& username);
        void returnItem(int itemId, const std::string& username);
        void waitUntilAvailable(int itemId, const std::string& username);
        std::string toString() const;

        friend std::ostream& operator<<(std::ostream& os, const InventoryManager& IM);//:
        //a more standard way of overloading the "<<" operator. no need for "getter" methods,
        //grants access to all private fields and methods for convenience.
        //-----
    };
}

#endif //INVENTORYMANAGER_H
