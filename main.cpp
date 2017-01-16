#include <iostream>
#include "PRobot.hpp"
#include "PGpioManager.hpp"

using namespace std;


int main(int argc, char *argv[])
{
    if(argc>1 && ((string)argv[1]=="-d"||(string)argv[1]=="-r"))
    {
        PGpioManager &pm=PGpioManager::getInstance();

        PRobot robot;
        robot.start();
        if((string)argv[1]=="-d")
            getchar();
        else if((string)argv[1]=="-r")
        {
            while(pm.read(Pin::SW1))
            {
                pm.write(Pin::LED1, true);
                this_thread::sleep_for(chrono::milliseconds(100));
                pm.write(Pin::LED1, false);
                this_thread::sleep_for(chrono::milliseconds(100));
            }
        }
        robot.stop();
    }
    else
    {
        cout << "Vous devez specifier un argument : "<<endl;
        cout <<"-d pour debug (le programme s'arrete avec un appui sur entrée)"<<endl;
        cout <<"-r pour release (le programme s'arrete avec un appui SW1)"<<endl;
    }
    return 0;
}

