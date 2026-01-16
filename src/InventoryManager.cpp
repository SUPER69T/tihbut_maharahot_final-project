#include "InventoryManager.hpp"
#include "IM_exception.hpp"
#include "Item.hpp"
#include <string>
#include <vector>
#include <mutex>
#include <condition_variable>

//Constructors:
InventoryManager::InventoryManager(){} //empty constructor.
InventoryManager::InventoryManager(const std::vector<Item>& items) : items(items){} //Item-type vector-reference receiving constructor.
//

std::string InventoryManager::listItems(){
    std::lock_guard<std::mutex> lock(mtx); //order precautionary measure.
    //both lock_guard and unique_lock unlock the mutex on scope exit(RAII mechanism).
    std::string result = "Item ID:    Name:    Available:    Borrowed by:    "; //going to require modular space tweaking.
    for(const auto& item : items){
        result += item.toString() + "\n";
    }
    return result;
}

void InventoryManager::borrowItem(const int itemId, const std::string& username){
    std::lock_guard<std::mutex> lock(mtx); 
    Item& founditem = findItemById(itemId);
    if(founditem.isAvailable()) founditem.borrow(username); //throws Item_exception.
}

void InventoryManager::returnItem(const int itemId, const std::string& username){
        std::unique_lock<std::mutex> lock(mtx);
        Item& founditem = findItemById(itemId);
        founditem.returnBack(username); //throws Item_exception.
        cv.notify_all();
}

void InventoryManager::waitUntilAvailable(const int itemId, const std::string& username){
    std::unique_lock<std::mutex> lock(mtx);
    Item& founditem = findItemById(itemId);
    cv.wait(lock, [&]{return founditem.isAvailable();}); //a nice little generic function as a boolean - 
    //checkup inside the cv to have safety against spurious wakeups...
    //the reason the lambda accepts a reference as a return is because the - isAvailable() method is a read-only method(const signature).
    founditem.borrow(username); //throws Item_exception.
}

Item& InventoryManager::findItemById(const int itemId){
    for(auto& item : items){
        if(item.getId() == itemId){
             return item;
        }
    }
    throw IM_exception("Item not found.");
}

        