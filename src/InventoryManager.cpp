#include "InventoryManager.hpp"
#include <iostream>

//Constructors:
InventoryManager::InventoryManager(){} //empty constructor.

InventoryManager::InventoryManager(const std::vector<Item>& items) : items(items){} //Item-type vector-reference receiving constructor.
//

std::string InventoryManager::listItems(){
    std::lock_guard<std::mutex> lock(mtx); //order precautionary measure.
    std::string result;
    for(const auto& item : items){
        result += item.toString() + "\n";
    }
    return result;
    std::lock_guard<std::mutex> unlock(mtx);
}

void InventoryManager::borrowItem(const int itemId, const std::string& username){
    std::lock_guard<std::mutex> lock(mtx); //the lock_guard will unlock the mutex when we exit the scope.
    Item& founditem = findItemById(itemId);
    if(founditem.isAvailable()) founditem.borrow(username);
    /*
    try{
        Item& founditem = findItemById(itemId);
        if(founditem.isAvailable()){
        founditem.borrow(username);
        }
        std::lock_guard<std::mutex> unlock(mtx);
    }
    catch (const std::runtime_error& e){
        std::cerr << e.what() << std::endl;
    }
    catch (const std::invalid_argument& e){
        std::cerr << e.what() << std::endl;
    }
    */
}

void InventoryManager::returnItem(const int itemId, const std::string& username){
        std::unique_lock<std::mutex> lock(mtx);
        Item& founditem = findItemById(itemId);
        if(founditem.getBorrower() != username){
            throw std::runtime_error("You are not the borrower of this item.");
        }
        founditem.returnBack(username);
        cv.notify_all();
}


void InventoryManager::waitUntilAvailable(const int itemId, const std::string& username){

}

Item& InventoryManager::findItemById(const int itemId){
    for(auto& item : items){
        if(item.getId() == itemId){
             return item;
        }
    }
    throw std::invalid_argument("Item not found.");
}

        