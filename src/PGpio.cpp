#include "PGpio.hpp"

using namespace std;
/*
PGpio::PGpio(const uint8_t pin, const bool direction)
{
    mDirection=direction;
    if(pin >= 4 && pin <= 27)
        mPin=pin;
    else
        mPin=0;

    mFile.open("/sys/class/gpio/export",ios::out);
    mFile<<to_string(mPin);
    mFile.close();
    mFile.open("/sys/class/gpio/gpio"+to_string(mPin)+"/direction",ios::out);
    if(mDirection)
        mFile<<"out";
    else
        mFile<<"in";
    mFile.close();

}

PGpio::~PGpio()
{
    mFile.open("/sys/class/gpio/unexport",ios::out);
    mFile<<to_string(mPin);
    mFile.close();
}

bool PGpio::read()
{
    bool value;
    mFile.open("/sys/class/gpio/gpio" + to_string(mPin) +"/value",ios::in);
    mFile>>value;
    mFile.close();
    return value;
}

void PGpio::write(const bool value)
{
    if(mDirection)
    {
        mFile.open("/sys/class/gpio/gpio"+to_string(mPin)+"/value",ios::out);
        mFile<<to_string(value);
        mFile.close();
    }
}*/

