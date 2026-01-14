#include "InventoryManager.hpp"
#include <iostream>

//Constructors:
InventoryManager::InventoryManager(){
    items.emplace_back(1, "Default Item");
    items.emplace_back(2, "Sample Item");
    // Populate with some default items on Main we will create our own items.
    
} //empty constructor.

InventoryManager::InventoryManager(const std::vector<Item>& items) : items(items){} //Item-type vector-reference receiving constructor.
//

std::string InventoryManager::listItems() const{
    std::string result;
    for(const auto& item : items){
        result += item.toString() + "\n";
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
        std::unique_lock<std::mutex> lock(mtx);
        Item& founditem = findItemById(itemId);
        if(founditem.getBorrower() != username){
            throw std::runtime_error("You are not the borrower of this item.");
        }
        founditem.returnBack(username);
        cv.notify_all();
}


void InventoryManager::waitUntilAvailable(int itemId, const std::string& username){

}

Item& InventoryManager::findItemById(int itemId){
    for(auto& item : items){
        if(item.getId() == itemId){
            // if(!item.isAvailable()){
            //     throw std::runtime_error("Item is currently borrowed.");
            // }
             return item;
        }
    }
    throw std::invalid_argument("Item not found.");
    
    
    
}

        