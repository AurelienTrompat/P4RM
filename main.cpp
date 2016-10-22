#include <iostream>
#include <unistd.h>
#include "PGpio.hpp"

using namespace std;

int main()
{
    PGpio led(26, true), sw(21, false);
    while(!sw.read())
    {
        led.write(1);
        usleep(100000);
        led.write(0);
        usleep(100000);
    }
    return 0;
}
