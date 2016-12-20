#include "PUltrasonicSensor.hpp"


using namespace std;

PUltrasonicSensor::PUltrasonicSensor(Pin echo , Pin trig) : mPm(PGpioManager::getInstance()), flag(false), tempreponse(0), go()
{
    //ctor
}

PUltrasonicSensor::~PUltrasonicSensor()
{
    //dtor
}

void PUltrasonicSensor::start(void)
{
    if (flag == 0)
    {
        flag = 1;
        go = thread(&PUltrasonicSensor::run, this);
    }
}

void PUltrasonicSensor::run(void)
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

void PUltrasonicSensor::stop(void)
{
    flag = 0;
    go.join();
}

int PUltrasonicSensor::temp(void)
{
    return (tempreponse);
}
