#include "PUltrasonicSensor.hpp"

<<<<<<< HEAD
using namespace std;

UltrasonicSensor::UltrasonicSensor(Pin echo , Pin trig) : mPm(PGpioManager::getInstance()), flag(false), tempreponse(0), go()
=======

UltrasonicSensor::UltrasonicSensor(PGpioManager &pm) : mPm(pm)
>>>>>>> origin/master
{
    //ctor
}

UltrasonicSensor::~UltrasonicSensor()
{
    //dtor
}

<<<<<<< HEAD
void UltrasonicSensor::start(void)
{
    if (flag == 0)
    {
        flag = 1;
        go = thread(&UltrasonicSensor::run, this);
    }
}

void UltrasonicSensor::run(void)
{
    float i = 0;
    while(flag)
    {
        chrono::high_resolution_clock::time_point debut = chrono::high_resolution_clock::now();
        mPm.write(Pin::TRIGGER_AV, true);

        while (mPm.read(Pin::ECHO_AV) == 0 && i < 0.001)
        {
            chrono::high_resolution_clock::time_point fin = chrono::high_resolution_clock::now();
            i = (debut - fin).count();
            if (i == 0.00001) mPm.write(Pin::TRIGGER_AV, false);
        }
        mPm.write(Pin::TRIGGER_AV, false);
        tempreponse = i*1000000;
        this_thread::sleep_for(chrono::milliseconds(60));
    }
}

void UltrasonicSensor::stop(void)
=======
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
>>>>>>> origin/master
{
    flag = 0;
    go.join();
}

<<<<<<< HEAD
int UltrasonicSensor::temp(void)
=======
int temp(void)
>>>>>>> origin/master
{
    return (tempreponse);
}
