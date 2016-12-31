#include "PI2C.hpp"
using namespace std;

PI2C::PI2C()
{
    //ctor
}

PI2C::~PI2C()
{
    //dtor
}

void PI2C::preRun()
{

}

void PI2C::run()
{

}

void PI2C::postRun()
{

}

void PI2C::handleCommand(const PCommand& command)
{
    if (command.mAgent == PCommand::Agent::I2C)
    {
        switch (command.mType)
        {
            case PCommand::Type::Test :
                {
                    cout << "test" << endl;
                    break;
                }

            default :
                break;
        }

    }
}
