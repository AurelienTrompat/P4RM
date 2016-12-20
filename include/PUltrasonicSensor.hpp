#ifndef PULTRASONICSENSOR_HPP
#define PULTRASONICSENSOR_HPP

#include "PGpioManager.hpp"
#include <atomic>
#include <thread>

class PUltrasonicSensor
{
    public:
        PUltrasonicSensor(Pin echo , Pin trig );
        PUltrasonicSensor(PGpioManager &pm, Pin echo , Pin triger );
        ~PUltrasonicSensor();
        void start(void);
        void stop(void);
        int temp(void);

    private:
        PGpioManager &mPm;
        std::atomic_bool flag;
        std::atomic_int tempreponse;
        std::thread go;

        void run(void);
};


#endif // PULTRASONICSENSOR_HPP
