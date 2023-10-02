//
// Created by jamer on 2023-09-29.
//

#ifndef GOMOKU_TIMER_H
#define GOMOKU_TIMER_H


#include <chrono>

class Timer
{
    using clock = std::chrono::steady_clock;
    clock::time_point tstart;
    clock::time_point tstop;
    double secs;

public:

    void start() {
        tstart = clock::now();
    }

    void stop() {
        tstop = clock::now();
        secs = std::chrono::duration<double>(tstop - tstart).count();
    }

    double seconds() const {
        return secs;
    }
};


#endif //GOMOKU_TIMER_H
