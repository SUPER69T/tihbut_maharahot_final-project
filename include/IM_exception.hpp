#ifndef INVENTORY_MANAGER_EXCEPTION_H
#define INVENTORY_MANAGER_EXCEPTION_H

#include <exception>
#include <string>
#include <vector>
#include <iostream>

namespace Store{

    class IM_exception : public std::exception{
    protected: //so that Item_exception can access this field.
        std::string msg;

    public:
        //inline: so that this "history" implementation shared memory isn't duplicated across -
        //multiple occurrences of Item_exception and doesn't cause a compilation error.
        inline static std::vector<std::string> history;


        //explicit: for clarity of intent -> requires the exception object initiation before passing a string.
        explicit IM_exception(const std::string& username, const std::string& message) : msg(username + ": " + message){
            history.push_back(username + ": " + message);
        }
        /*
        virtual: because "what()" method is declared as a virtual method in the base -
        class, as to be overloadable by other classes to enable polymorphism.

        const char: so the string never gets modified(for safety).

        const function: enforces const on the object which calls "what".

        noexcept override: google: "The noexcept specifier in C++ is -
        a declaration that guarantees a function will not throw any exceptions".

        char*: the main 2 reasons for returning a pointer is for the possibility of the message being a -
        primitive type, and because it was declared so in the original exception base class this way...  
        */
        virtual const char* what() const noexcept override{ //never actually used. 
            return msg.c_str();
        };

        static void print_history(){
            std::cout << "---InventoryManager_exceptions_history---\n"
            "Username:       Message:\n\n" << std::endl;

            for (const auto& error : history){
                std::cout << "-" << error << '\n';
            }
            std::cout << std::flush;
        }
    };
}

#endif //INVENTORY_MANAGER_EXCEPTION_H