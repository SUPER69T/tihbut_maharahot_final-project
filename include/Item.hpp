#ifndef ITEM_H
#define ITEM_H

#include <string>

class Item {
    private:
        int id;
        std::string name;
        bool isBorrowed;
        std::string borrowedBy;

    public:
        Item(int id,const std::string& name);
        Item();

        int getId() const;
        std::string& getName() ;
        bool isAvailable() const;
        std::string& getBorrower();

        void borrow(const std::string& username);
        void returnBack(const std::string& username);
        std::string toString() const;
};

std::ostream& operator<<(std::ostream& os, const Item& item);//:
//implemented here because of the Open-Closed Principle and the way Argument Dependent Lookup (ADL) works.

#endif //ITEM_H