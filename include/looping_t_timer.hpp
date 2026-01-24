#ifndef LOOPING_T_TIMER_HPP
#define LOOPING_T_TIMER_HPP

#include <chrono>
#include <string>

class looping_t_timer{
    private:
            std::chrono::steady_clock::time_point deadline;
            std::chrono::milliseconds duration;
            std::string name;

    public:
        //constructor:
        looping_t_timer(const std::chrono::seconds& timeout, const std::string& timer_name);
        //

        //destructor:
        ~looping_t_timer(); 
        //
        
        void check_timeout() const;
        void reset();
};

#endif //LOOPING_T_TIMER_HPP