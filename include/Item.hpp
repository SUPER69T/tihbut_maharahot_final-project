#ifndef ITEM_H
#define ITEM_H

#include <string>

namespace Store{

    class Item {
        private:
            int id;
            std::string name;
            bool isBorrowed;
            std::string borrowedBy;

        public:
            Item(int id,const std::string& name);
            Item();

            //---
            //by default: primitives return copies, no need for "const" return modifier:
            int getId() const;
            bool isAvailable() const;
            //by default: objects return references, a const function contract requires a const return type:
            const std::string& getName() const;
            const std::string& getBorrower() const;
            //---
            
            void borrow(const std::string& username);
            void returnBack(const std::string& username);
            std::string toString() const;
    };

    std::ostream& operator<<(std::ostream& os, const Item& item);//:
    //implemented here because of the Open-Closed Principle and the way Argument Dependent Lookup (ADL) works.
}

#endif //ITEM_H