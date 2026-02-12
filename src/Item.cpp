#include "Item.hpp"
#include "Item_exception.hpp"
#include <string>
#include <iomanip> //for: std::setw.
#include <sstream> //for: std::ostringstream.

namespace Store{

    //Constructors:
    Item::Item(int id,const std::string& name): id(id),name(name),isBorrowed(false),borrowedBy(""){
        //if the next code is thrown c++ will automatically cleanup the uninitiated Item object's fields. 
        if(id < 0){
            throw Item_exception("<NULL>", name, "'s ID should be possitive!!");
        }
    }
    Item::Item(): id(0),name("0"),isBorrowed(false),borrowedBy(""){}
    //

    //Public methods:
    int Item::getId() const{
        return id;
    }
    bool Item::isAvailable() const{
        return !isBorrowed;
    }
    const std::string& Item::getName() const{
        return name;
    }
    const std::string& Item::getBorrower() const{
        return borrowedBy;
    }
    void Item::borrow(const std::string& username){
        if(isBorrowed){
            throw Item_exception(username, name, " is not available.");
        }
        borrowedBy = username;
        isBorrowed = true;
    }

    void Item::returnBack(const std::string& username){
        if(!isBorrowed){
            throw Item_exception(username, name, " is not borrowed.");
        }
        if(borrowedBy != username){
            throw Item_exception(username, name, " has a different borrower.");
        }
        isBorrowed = false;
        borrowedBy = "";
    }
    //dynamic padding:
    //---
    std::string Item::toString() const{
        std::ostringstream oss;
        oss << std::left; //setting the global alignment to left for the entire row.
        //formatted width sizes: ID (10) | Name (20) | Available (12) | Borrower (20).
        oss << std::setw(10) << id 
        << std::setw(20) << name 
        << std::setw(12) << (isBorrowed ? "No" : "Yes")
        << (isBorrowed ? borrowedBy : "Free") 
        << "\n";
        return oss.str();
    } 
    //---

    std::ostream& operator<<(std::ostream& os, const Item& item) {
        os << item.toString(); //Reusing our previous toString method like python's: __str__/__repr__ differences.
        return os;
    }
    //
}






