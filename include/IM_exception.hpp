#ifndef INVENTORY_MANAGER_EXCEPTION_H
#define INVENTORY_MANAGER_EXCEPTION_H

#include <exception>
#include <string>
#include <vector>
#include <iostream>

//custom exceptions implementation explained at: Item_exception.hpp.
class IM_exception : public std::exception {
private:
    std::string msg;

public:
    inline static std::vector<std::string> history;

    explicit IM_exception(const std::string& message) : msg(message) {
        history.push_back(message);
    }

    virtual const char* what() const noexcept override{
        return msg.c_str();
    };

    static void print_history() {
        std::cout << "--- Item exceptions history ---" << std::endl;
        for(const auto& error : history){
            std::cout << "- " << error << std::endl;
        }
    }
};

#endif