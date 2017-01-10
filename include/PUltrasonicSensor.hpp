#ifndef PULTRASONICSENSOR_HPP
#define PULTRASONICSENSOR_HPP

#include "PGpioManager.hpp"
#include <atomic>
#include <thread>

class PUltrasonicSensor
{
    public:
        PUltrasonicSensor();
        ~PUltrasonicSensor();

    private:
        void preRun();
        void run();
        void postRun();
};


#endif // PULTRASONICSENSOR_HPP
