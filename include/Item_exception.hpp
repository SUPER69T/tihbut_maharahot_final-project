#ifndef ITEM_EXCEPTION_H
#define ITEM_EXCEPTION_H

#include "IM_exception.hpp" //Inheriting from IM_exception.

namespace Store{

    //custom exceptions implementation explained at: IM_exception.hpp.
    class Item_exception : public IM_exception{
        public:
        explicit Item_exception(const std::string& username, const std::string& Item_name, const std::string& message) 
        : IM_exception(username, "Item_exception: " + Item_name + message) {}
    };
}
#endif //ITEM_EXCEPTION_H   