#include "PUltrasonicSensor.hpp"


UltrasonicSensor::UltrasonicSensor(PGpioManager &pm) : mPm(pm)
{
    //ctor
}

UltrasonicSensor::~UltrasonicSensor()
{
    //dtor
}

void start(void)
{
    flag = 1;
    go = std::thread(run);
}

void run(void)
{
    int i;
    while(flag)
    {
        i = 0;
        pm.write(Pin::triger, true);
        while (Pin::echo == 0 && i < 1000)
        {
            if (i = 10) pm.write(Pin::triger, false);
            std::Thread::Sleep(1);
            i++;
        }
        if (pm.read(Pin::triger, true)) pm.write(Pin::triger, false);
        std::Thread::Sleep(15);
        tempreponse = i;
    }
}

void stop(void)
{
    flag = 0;
    go.join();
}

int temp(void)
{
    return (tempreponse);
}
