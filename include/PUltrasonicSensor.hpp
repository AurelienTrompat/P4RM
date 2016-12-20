#ifndef ULTRASONICSENSOR_HPP
#define ULTRASONICSENSOR_HPP

#include "PGpioManager.hpp"
#include <atomic>
#include <thread>

class UltrasonicSensor
{
    public:
        UltrasonicSensor(Pin echo , Pin trig );
        UltrasonicSensor(PGpioManager &pm, Pin echo , Pin triger );
        ~UltrasonicSensor();
        void start(void);
        void stopUltrasonicSensor::(void);
        int temp(void);

    private:
        PGpioManager &mPm;
        std::atomic_bool flag;
        std::atomic_int tempreponse;
        std::thread go;

        void run(void);
};

#endif // ULTRASONICSENSOR_HPP
