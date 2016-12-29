#include <iostream>
#include <exception>

#include "SFML/System.hpp"

#include "PGpioManager.hpp"
#include "PUltrasonicSensor.hpp"
#include "PNetwork.hpp"

using namespace std;

int main()
{
    try
    {
        PGpioManager &pm = PGpioManager::getInstance();
        pm.declarePin(Pin::LED1);
        pm.declarePin(Pin::SW1);
        pm.declarePin(Pin::ECHO_AV);
        pm.declarePin(Pin::TRIG_AV);

        PNetwork testReseau;
        testReseau.start();

        /*PUltrasonicSensor test(pm,Pin::ECHO_AV, Pin::TRIG_AV);
        test.start();

        while(pm.read(Pin::SW1))
        {
            if (test.temp() > 296) pm.write(Pin::LED1, true);
            if (test.temp() > 2) pm.write(Pin::LED1, true);
            else pm.write(Pin::LED1, false);
            this_thread::sleep_for(chrono::milliseconds(200));
        }

        test.stop();*/
        getchar();
        testReseau.stop();
    }
    catch(exception const& exep)
    {
        cerr << exep.what() <<endl;
    }
    return 0;
}
