#include "InventoryManager.hpp"
#include "IM_exception.hpp"
#include "Item_exception.hpp"
#include "Item.hpp"
#include <string>
#include <vector> //:
//would have used #include <unordered_map> instead of the <vector>...
#include <mutex>
#include <condition_variable>

namespace Store{

    int InventoryManager::total_IMs = 0;

    //Constructors:
    InventoryManager::InventoryManager(){ //empty constructor.
        total_IMs++;
        this->IM_Id = total_IMs;
    } 
    InventoryManager::InventoryManager(const std::vector<Item>& items) : items(std::move(items)){} //Item-type vector-reference receiving constructor.
    //std::move: a move semantic
    //

    //Private methods:
    Item& InventoryManager::findItemById(const std::string& username, const int itemId){
        //no need for another lock, all methods that are using this method should have already locked the mutex themselves.
        for(auto& item : items){
            if(item.getId() == itemId){
                return item;
            }
        }
        throw IM_exception(username, "Item not found.");
    }
    //

    //Public methods:
    std::string InventoryManager::listItems(){
        std::lock_guard<std::mutex> lock(mtx); //order precautionary measure.
        //both lock_guard and unique_lock unlock the mutex on scope exit(RAII mechanism).
        std::string result = "Item ID:    Name:    Available:    Borrowed by:    "; //going to require dynamic space tweaking.
        for(const auto& item : items){
            result += item.toString() + "\n";
        }
        return result;
    }

    void InventoryManager::borrowItem(const int itemId, const std::string& username){
        std::lock_guard<std::mutex> lock(mtx); 
        Item& founditem = findItemById(username, itemId);
        if(founditem.isAvailable()) founditem.borrow(username); //throws Item_exception.
    }

    void InventoryManager::returnItem(const int itemId, const std::string& username){
            std::lock_guard<std::mutex> lock(mtx);
            Item& founditem = findItemById(username, itemId);
            try{
                founditem.returnBack(username); //throws Item_exception.
            }
            catch (const Item_exception& e){ //catching e by reference and ensuring no modification of the exception object.
                cv.notify_all();
                throw; //rethrows - e exception object with all of it's parameters intact.
            }
            cv.notify_all();
    }

    void InventoryManager::waitUntilAvailable(const int itemId, const std::string& username){
        std::unique_lock<std::mutex> lock(mtx);
        Item* itemPtr = nullptr; //use of a pointer instead of a reference for the purpose of -
        //reusing that same Item()-object memory-address in the case of borrowing the item. 
        cv.wait(lock, [&]{
            itemPtr = &findItemById(username, itemId);
            return itemPtr->isAvailable();
        }); //a nice little generic function as a boolean checkup inside the cv to have safety against - 
        //spurious wakeups(the C++ standard library internally wraps the wait in a while loop)...
        //
        //gemini came up with the idea to both find the item and check whether it is - 
        //available inside the generic function as the cv boolean checup itself...
        //
        //the reason the lambda accepts a reference as a return is because the - 
        //isAvailable() method is a read-only method(const signature).
        itemPtr->borrow(username); //throws Item_exception.
    }
    //

    std::ostream& operator<<(std::ostream& os, const InventoryManager& IM){
        os << "InventoryManager, ID - " + std::to_string(IM.IM_Id) + ".";
        return os;
    }
}