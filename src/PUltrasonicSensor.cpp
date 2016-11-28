#include "PUltrasonicSensor.hpp"

UltrasonicSensor::UltrasonicSensor(PGpioManager &pm, Pin id) : mPm(pm), mPin(id)
{
    //ctor
}

UltrasonicSensor::~UltrasonicSensor()
{
    //dtor
}
