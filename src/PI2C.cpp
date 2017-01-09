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
    OpenI2C();
    SetAdresse(0x10);
   // int result = i2c_smbus_write_byte_data(fd, 0x0f, 0x64);
}

void PI2C::run()
{

}

void PI2C::postRun()
{
    close(fd);
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

void PI2C::SendEvent(PEvent::Type typeEvent)
{
    PEvent event;
    event.mType=typeEvent;
    pushEvent(event);
}

void PI2C::OpenI2C()
{

     if ((fd = open("/dev/i2c-1",O_RDWR))== -1)
    {
        SendEvent(PEvent::Type::I2C_NotOpen);
    }
    else
    {
        SendEvent(PEvent::Type::I2C_Open);
    }

}
void PI2C::SetAdresse(uint8_t adresse)
{
    if (ioctl(fd, I2C_SLAVE, adresse)== -1)
    {
        SendEvent(PEvent::Type::I2C_SetAdresseFailed);
    }
    else
    {
        SendEvent(PEvent::Type::I2C_SetAdresseSuccess);
    }
}

