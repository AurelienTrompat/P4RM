#ifndef ULTRASONICSENSOR_HPP
#define ULTRASONICSENSOR_HPP

#include "PGpioManager.hpp"
<<<<<<< HEAD
#include <atomic>
#include <thread>

=======
#include <Atomic>
#include <thread>
>>>>>>> origin/master

class UltrasonicSensor
{
    public:
<<<<<<< HEAD
        UltrasonicSensor(Pin echo , Pin trig );
=======
        UltrasonicSensor(PGpioManager &pm, Pin echo , Pin triger );
>>>>>>> origin/master
        ~UltrasonicSensor();
        void start(void);
        void stop(void);
        int temp(void);

    private:
        PGpioManager &mPm;
        std::atomic_bool flag;
<<<<<<< HEAD
        std::atomic_int tempreponse;
        std::thread go;
=======
        int tempreponse;
        std::thread go;

        void run(void);

>>>>>>> origin/master

        void run(void);
};

#endif // ULTRASONICSENSOR_HPP
