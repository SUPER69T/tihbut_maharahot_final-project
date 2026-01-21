#include <stdexcept>
#include <string>
#include <cstring> //used for strerror.

//gemini's help for making a hirarchy-exceptions structure:
//{
//Base Network Exception:
//children: Bind_Exception, Timeout_Exception.
class Network_Exception : public std::runtime_error{
    protected:
        int error_code; //used to save-in-place the error code errno returns, a good idea for multi-threading exception implementation.

    public:
        explicit Network_Exception(const std::string& msg, int err) //input: (message + the compiler's error code).
        : std::runtime_error(msg + ": " + std::strerror(err)), //:
        //sterror is an awsome C function that returns the text description of the error types, requires an error-code.
        error_code(err) {}

        //implemented mainly for the practice itself:
        int get_code() const noexcept {return error_code;} //a method used for manual errno if-condition checking in a "catch" block where the exception is -
        //thrown in order to apply an alternative operation rather than Network_Exception's standard handling(implemented in - server.cpp, line ~ 110).
    };

//a Network_Exception's child- "Bind" Error:
class Bind_Exception : public Network_Exception{
    public:
        explicit Bind_Exception(const std::string& msg, int err) 
        : Network_Exception("Bind Error: " + msg, err) {}
    };

//a Network_Exception's child- "Timeout" Error:
class Timeout_Exception : public Network_Exception{
    public:
        explicit Timeout_Exception(const std::string& msg, int err) 
        : Network_Exception("Timeout Error: " + msg, err) {}
    };
//}
