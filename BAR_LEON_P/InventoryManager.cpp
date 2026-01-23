#include "InventoryManager.hpp"
#include <iostream>

//Constructors:
InventoryManager::InventoryManager(){
    // 
    items.emplace_back(1, "Camera");
    items.emplace_back(2, "Tripod");
    items.emplace_back(3, "Laptop");
    items.emplace_back(4, "Projector");
    items.emplace_back(5, "Microphone");
    items.emplace_back(6, "Speaker");
    items.emplace_back(7, "HDMI_Cable");
    items.emplace_back(8, "Ethernet_Cable");
    items.emplace_back(9, "Keyboard");
    items.emplace_back(10, "Mouse");
    items.emplace_back(11, "Monitor");
    items.emplace_back(12, "USB_Hub");
    items.emplace_back(13, "Power_Bank");
    items.emplace_back(14, "Router");
    items.emplace_back(15, "VR_Headset");
    
    
} //empty constructor.

InventoryManager::InventoryManager(const std::vector<Item>& items) : items(items){} //Item-type vector-reference receiving constructor.
//

std::string InventoryManager::listItems() {
    std::lock_guard<std::mutex> lock(mtx);
    std::string result;
    result += "OK LIST " + std::to_string(items.size()) + "\n";
    for(const auto& item : items){
        result += item.toString() + "\n";
       
    }
    return result;
}

void InventoryManager::borrowItem(int itemId, const std::string& username){
    std::unique_lock<std::mutex> lock(mtx);
        Item& founditem = findItemById(itemId);
        if(!founditem.isAvailable()){
            throw std::runtime_error("ERR UNAVAILABLE borrowed_by="+founditem.getBorrower()+"\n");
        }
        if(username.empty()){
            throw std::invalid_argument("Username cannot be empty");
        }
        founditem.borrow(username);
}

void InventoryManager::returnItem(int itemId, const std::string& username){
        std::unique_lock<std::mutex> lock(mtx);
        Item& founditem = findItemById(itemId);
        if(founditem.getBorrower() != username){
            throw std::runtime_error("ERR PERMISSION not_owner \n");
        }
        founditem.returnBack(username);
        cv.notify_all();
}


void InventoryManager::waitUntilAvailable(int itemId, const std::string& username){
    std::unique_lock<std::mutex> lock(mtx);
   if (username.empty()) {
        throw std::invalid_argument("Username cannot be empty.");
    }
    Item& foundItem=findItemById(itemId); 
    if(username==foundItem.getBorrower()){
        throw std::runtime_error("ERR DEADLOCK item");
    }
    cv.wait(lock, [&foundItem] { return foundItem.isAvailable(); });
    
}


Item& InventoryManager::findItemById(int itemId) {
    for(auto& item : items){
        if(item.getId() == itemId){
           
             return item;
        }
    }
    throw std::runtime_error(" ERR NOT_FOUND item");
    
    
    
}

        