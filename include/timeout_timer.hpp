#ifndef TIMEOUT_TIMER_HPP
#define TIMEOUT_TIMER_HPP

#include <chrono>
#include <string>

class timeout_timer{
    private:
            std::chrono::steady_clock::time_point deadline;
            std::chrono::milliseconds duration;
            std::string name;

    public:
        //constructor:
        timeout_timer(const std::chrono::seconds& timeout, const std::string& timer_name);
        //
        void check_timeout() const;
        void reset();
};

#endif //TIMEOUT_TIMER_H