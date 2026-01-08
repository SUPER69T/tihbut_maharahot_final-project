#include "InventoryManager.h";

InventoryManager(Item& items): items(items){

    std::string listItems();

    void borrowItem(int itemId, const std::string& username){
        try:
            Item item = findItemById(itemId);
            
        //catch(nonexistant_item_err e){}
    }

    void returnItem(int itemId, const std::string& username){

    }

    void waitUntilAvailable(int itemId, const std::string& username){

    }

    Item& findItemById(int itemId){
        for (size_t i = 0 ; i < items.size() ; i++){
            if(item[i] -> name == itemId) return item[i];
        }
    } 
}
        