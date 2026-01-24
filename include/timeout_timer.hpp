#ifndef TIMEOUT_TIMER_HPP
#define TIMEOUT_TIMER_HPP

#include <chrono>
#include <string>

class timeout_timer{
    private:
            std::chrono::steady_clock::time_point deadline;
            std::string name;

    public:
        //constructor:
        timeout_timer(std::chrono::milliseconds& timeout, const std::string& timer_name);
        //
        void check_timeout() const;
};

#endif //TIMEOUT_TIMER_H