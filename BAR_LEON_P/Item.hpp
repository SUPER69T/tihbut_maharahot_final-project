#ifndef Item_h
#define Item_h
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
#endif