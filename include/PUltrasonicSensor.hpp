#ifndef ULTRASONICSENSOR_HPP
#define ULTRASONICSENSOR_HPP

#include "PGpioManager.hpp"


class UltrasonicSensor
{
    public:
        UltrasonicSensor(PGpioManager &pm, Pin id);
        ~UltrasonicSensor();

    private:
        PGpioManager &mPm;
        Pin mPin;

};

#endif // ULTRASONICSENSOR_HPP
