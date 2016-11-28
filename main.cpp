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

        while(!pm.read(Pin::SW1))
        {
            pm.write(Pin::LED1, true);
            sf::sleep(sf::milliseconds(200));
            pm.write(Pin::LED1, false);
            sf::sleep(sf::milliseconds(200));
        }
    }
    catch(exception const& exep)
    {
        cerr << exep.what() <<endl;
    }

    return 0;
}
