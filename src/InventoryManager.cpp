#include "InventoryManager.hpp"
#include "IM_exception.hpp"
#include "Item_exception.hpp"
#include "Item.hpp"
#include <string>
#include <vector> //:
//would have used #include <unordered_map> instead of the <vector>...
#include <mutex>
#include <condition_variable>
#include <iomanip> //for: std::setw.
#include <sstream> //for: std::ostringstream.

namespace Store{

    int total_IMs = 0;
    
    //Constructors:
    InventoryManager::InventoryManager(){ //empty constructor.
        total_IMs++;
        this->IM_Id = total_IMs;
    } 
    InventoryManager::InventoryManager(std::vector<Item> items) : items(std::move(items)){} //Item-type-vector copy-constructor.
    //std::move: a move semantic - used to pass resources more efficiently - as an rvalue, instead of copying them.
    //

    //Private methods:
    Item& InventoryManager::findItemById(const std::string& username, const int itemId){ //:
        //itemId is taken by value: in cpp int cannot be null.
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
        //
        //printing the header:
        std::ostringstream header;
        header << std::left 
            << std::setw(10) << "ID" 
            << std::setw(20) << "Name" 
            << std::setw(12) << "Available" 
            << "Borrowed By" << "\n";
        header << std::string(60, '-') << "\n"; 
        //
        std::string result = header.str(); //result is the output string.
        result.reserve(result.size() + items.size() * 60); //alocating result's internal buffer for all upcoming Item-toStrings.
        for(const auto& item : items){
            result += item.toString();
        }
        return result;
    }

    void InventoryManager::borrowItem(const int itemId, const std::string& username){
        std::lock_guard<std::mutex> lock(mtx); 
        Item& founditem = findItemById(username, itemId);
        founditem.borrow(username); //throws Item_exception.
    }

    void InventoryManager::returnItem(const int itemId, const std::string& username){
            std::lock_guard<std::mutex> lock(mtx);
            try{
                Item& founditem = findItemById(username, itemId); //throws Item_exception.
                founditem.returnBack(username); //throws Item_exception.
            }
            catch(const Item_exception& e){ //catching e by reference and ensuring no modification of the exception object.
                throw; //rethrows - e exception object with all of it's parameters intact.
            }
            cv.notify_all();
    }

    void InventoryManager::waitUntilAvailable(const int itemId, const std::string& username){
        std::unique_lock<std::mutex> lock(mtx);
        try{
            Item* itemPtr = &findItemById(username, itemId); //use of a pointer instead of a reference for the purpose of -
            //reusing that same Item()-object memory-address in the case memory changes.
            if(itemPtr->getBorrower() == username) throw IM_exception(username, "The user already has a hold of this item.");
            
            cv.wait(lock, [&]{ //a cute generic function as a boolean checkup inside the cv to have safety against spurious wakeups.
                itemPtr = &findItemById(username, itemId);
                return itemPtr->isAvailable();
            });
            //
            //gemini came up with the idea to both find the item and check whether it is - 
            //available inside the generic function as the cv boolean checup itself...
            //
            //[&] = lambda accepts all variables in the scope by reference.
            //  
        }
        //-----
        catch(const Item_exception& e){ 
                //cv.notify_all(); //:
                //this operation is redundant, since the RAII lock itself will release the - 
                //mutex in the case of exitting the scope via an exception throw.
                throw; //rethrows.
        }
        //-----
    }
    //

    std::ostream& operator<<(std::ostream& os, const InventoryManager& IM){
        os << "InventoryManager, ID - " + std::to_string(IM.IM_Id) + ".";
        return os;
    }
}