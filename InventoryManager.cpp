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
    try{
        Item item = findItemById(itemId);
    }
    catch (const std::invalid_argument& e){
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

void InventoryManager::returnItem(int itemId, const std::string& username){
    
}

void InventoryManager::waitUntilAvailable(int itemId, const std::string& username){

}

Item& InventoryManager::findItemById(int itemId){
    for (size_t i = 0 ; i < items.size() ; i++){
        if(items[i].getId() == itemId) return items[i];
    }
    throw std::invalid_argument("Item not found.");
}

        