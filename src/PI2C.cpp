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


}

void PI2C::run()
{

}

void PI2C::postRun()
{
    close(mFd);
}

void PI2C::handleCommand(const PCommand& command)
{
    if (command.mAgent == PCommand::Agent::I2C)
    {
        switch (command.i2c_p.type)
        {
            case PCommand::I2C_Parameters::I2C_Command::SetCommandMotor :
            {
                WriteData(command.i2c_p);
                break;
            }
            case PCommand::I2C_Parameters::I2C_Command::StopMoteur :
            {

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

     if ((mFd = open("/dev/i2c-1",O_RDWR))== -1)
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
    if (ioctl(mFd, I2C_SLAVE, adresse)== -1)
    {
        SendEvent(PEvent::Type::I2C_SetAdresseFailed);
    }
    else
    {
        SendEvent(PEvent::Type::I2C_SetAdresseSuccess);
    }
}

int PI2C::BusAccess (bool rw, uint8_t command, int dataSize, union i2c_smbus_data *data)
{
    struct i2c_smbus_ioctl_data args ;

    args.read_write = rw ;
    args.command    = command ;
    args.size       = dataSize ;
    args.data       = data ;
    return ioctl (mFd, I2C_SMBUS, &args) ;
}

void PI2C::WriteData(const struct PCommand::I2C_Parameters &i2c_p)
{
    union i2c_smbus_data data;
    uint8_t cmd;

    cmd = i2c_p.motorP.RAZdefaultGauche << 5;
    data.byte=i2c_p.motorP.vitesseGauche;

    SetAdresse(0x10);
    BusAccess(I2C_SMBUS_WRITE,cmd,I2C_SMBUS_BYTE_DATA, &data);
}
