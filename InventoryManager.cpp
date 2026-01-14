#include "InventoryManager.hpp"
#include <iostream>

//Constructors:
InventoryManager::InventoryManager(){} //empty constructor.

InventoryManager::InventoryManager(const std::vector<Item>& items) : items(items){} //Item-type vector-reference receiving constructor.
//

std::string InventoryManager::listItems(){
    std::string result;
    for(const auto& item : items){
        result += std::to_string(item.getId()) + " ";
    }
    return result;
}

void InventoryManager::borrowItem(int itemId, const std::string& username){
    std::unique_lock<std::mutex> lock(mtx);
    try{
        Item founditem = findItemById(itemId);
        founditem.borrow(username);
        lock.unlock();
        cv.notify_all();
    }
    catch (const std::runtime_error& e){
        std::cerr << e.what() << std::endl;
    }
    catch (const std::invalid_argument& e){
        std::cerr << e.what() << std::endl;
    }
}

void InventoryManager::returnItem(int itemId, const std::string& username){
    
}

void InventoryManager::waitUntilAvailable(int itemId, const std::string& username){

}

Item& InventoryManager::findItemById(int itemId){
    for(auto& item : items){
        if(item.getId() == itemId){
            if(!item.isAvailable())]){
                throw std::runtime_error("Item is currently borrowed.");
            }
             return item;
        }
    throw std::invalid_argument("Item not found.");
    }
}

        