#include "Item.hpp"
#include <Item_exception.hpp>
#include <string>
//#include <iostream>

Item::Item(int id,const std::string& name): id(id),name(name),isBorrowed(false),borrowedBy(""){
    if(id < 0){
        throw Item_exception("ID should be possitive");
    }
}
Item::Item(): id(0),name("0"),isBorrowed(false),borrowedBy(""){}
int Item::getId() const{
    return id;
}
std::string& Item::getName() {
    return name;
}
bool Item::isAvailable() const{
    return !isBorrowed;
}
std::string& Item::getBorrower(){
    return borrowedBy;
}
void Item::borrow(const std::string& username){
    if(isBorrowed){
        throw Item_exception("The item is not available");
    }

    else if(typeid(username) != typeid(std::string) or typeid(username) != typeid(char)){ //guessing the request was to simply check username type compatibility...?
        throw Item_exception("Invalid username.");
    }

    borrowedBy = username;
    isBorrowed = true;
}

void Item::returnBack(const std::string& username){
    if(!isBorrowed){
        throw Item_exception("The item is not borrowed.");
    }
    if(borrowedBy != username){
        throw Item_exception("You are not the borrower of this item.");
    }
    
    isBorrowed = false;
    borrowedBy = "";
}
std::string Item::toString() const{
    return std::to_string(id) + name + (isBorrowed ? "No" : "Yes") + (isBorrowed ? borrowedBy : "");
}

std::ostream& operator<<(std::ostream& os, const Item& item) {
    os << item.toString(); //Reusing our previous toString method like python's: __str__/__repr__ differences.
    return os;
}






