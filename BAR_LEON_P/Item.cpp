#include "Item.hpp"
#include <iostream>
#include <stdexcept>

// using namespace std;

Item::Item(int id,const std::string& name): id(id),name(name),isBorrowed(false),borrowedBy(""){
    if(id<0){
        throw std::invalid_argument("ID should be possitive");
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
    if (isBorrowed){
        throw std::runtime_error("The item is not available");
    }
    borrowedBy=username;
    isBorrowed=true;
}

void Item::returnBack(const std::string& username){
    if(!isBorrowed){
        throw std::runtime_error("The item is not borrowed.");
    }
    if(borrowedBy != username){
        throw std::runtime_error("You are not the borrower of this item.");
    }
    
    isBorrowed=false;
    borrowedBy="";
}
std::string Item::toString() const{
    return std::to_string(id) + " " + name + " " + (isBorrowed ? "BORROWED by= "+borrowedBy : "FREE");
}







