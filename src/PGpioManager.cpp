#include "PGpioManager.hpp"

using namespace std;

PGpioManager::PGpioManager()
{

}

PGpioManager::~PGpioManager()
{
    //dtor
}

void PGpioManager::addPin(Pin id, const uint8_t pin)
{
    mPinMap.insert(make_pair(id, pin));
}

void PGpioManager::removePin(Pin id)
{
    auto found = mPinMap.find(id);
    if(found != mPinMap.end())
    {
        mPinMap.erase(found);
    }

}
bool PGpioManager::read(Pin id)
{
    switch(id)
    {
    case Pin::LED1:
        return 0;
    case Pin::SW1:
        return 1;
    default:
        return 0;
    }
}

