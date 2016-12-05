#include <iostream>
#include <exception>
#include "PGpioManager.hpp"
#include "SFML/System.hpp"

using namespace std;

int main()
{
    try
    {
        PGpioManager &pm = PGpioManager::getInstance();
        pm.declarePin(Pin::LED1);
        pm.declarePin(Pin::SW1);

        pm.declarePin(Pin::ECHO_AV);   // rgzgrzergzergzerg
        pm.declarePin(Pin::TRIGGER_AV); // zrgzergzergzergzerg

        UltrasonicSensor test(pm,Pin::ECHO_AV, Pin::TRIGGER_AV);
        test.start();

        while(!pm.read(Pin::SW1))
        {
            if (test.temp() > 2) pm.write(Pin::LED1, true);
            else pm.write(Pin::LED1, false);
            sf::sleep(sf::milliseconds(200));
        }
    }
    catch(exception const& exep)
    {
        cerr << exep.what() <<endl;
    }

    return 0;
}
