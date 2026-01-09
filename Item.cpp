#include "Item.hpp"
#include <iostream>
#include <stdexcept>

// using namespace std;

Item::Item(int id,const std::string& name): id(id),name(name),isBorrowed(false),borrowedBy(""){
    if(id<0){
        throw ("ID should be possitive");
    }
}
Item::Item(): id(0),name("0"),isBorrowed(false),borrowedBy(""){}
int Item::getId() const{
    return id;
}
std::string& Item::getName() {
    return name;
}
bool Item::isAvailable(){
    return !isBorrowed;
}
std::string& Item::getBorrower(){
    return borrowedBy;
}
void Item::borrow(const std::string& username){
    if (isBorrowed){
        throw std::runtime_error("The item is not Avaliable");
    }
    borrowedBy=username;
    isBorrowed=true;
}







