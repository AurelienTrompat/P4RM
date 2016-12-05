#ifndef ULTRASONICSENSOR_HPP
#define ULTRASONICSENSOR_HPP

#include "PGpioManager.hpp"
#include <Atomic>
#include <thread>

class UltrasonicSensor
{
    public:
        UltrasonicSensor(PGpioManager &pm, Pin echo , Pin triger );
        ~UltrasonicSensor();
        void start(void);
        void stop(void);
        int temp(void);

    private:
        PGpioManager &mPm;
        std::atomic_bool flag;
        int tempreponse;
        std::thread go;

        void run(void);


};

#endif // ULTRASONICSENSOR_HPP
