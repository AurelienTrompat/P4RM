#include <iostream>
#include <unistd.h>
#include <exception>
#include "PGpioManager.hpp"

using namespace std;

int main()
{
    try
    {
        PGpioManager pm;
        pm.addPin(Pin::LED1);
        pm.addPin(Pin::SW1);

        while(!pm.read(Pin::SW1))
        {
            pm.write(Pin::LED1, true);
            usleep(100000);
            pm.write(Pin::LED1, false);
            usleep(100000);
        }
    }
    catch(exception const& exep)
    {
        cerr << exep.what() <<endl;
    }

    return 0;
}
