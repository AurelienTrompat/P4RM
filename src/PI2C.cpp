#include "PI2C.hpp"

using namespace std;

PI2C::PI2C() : mNewCommand(false)
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
                MicroC_WriteCmd(command.i2c_p);
                break;
            }
            case PCommand::I2C_Parameters::I2C_Command::StopMoteur :
            {
                MicroC_ShutdownMoteur();
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

}
void PI2C::SetAdresse(uint8_t adresse)
{
    if (ioctl(mFd, I2C_SLAVE, adresse)== -1)
    {
        SendEvent(PEvent::Type::I2C_SetAdresseFailed);
    }
}

void PI2C::BusAccess (bool rw, uint8_t command, int dataSize, union i2c_smbus_data *data)
{
    struct i2c_smbus_ioctl_data args ;

    args.read_write = rw ;
    args.command    = command ;
    args.size       = dataSize ;
    args.data       = data ;

    if(ioctl (mFd, I2C_SMBUS, &args) == -1)
        SendEvent(PEvent::Type::I2C_WriteFailed);
}

void PI2C::MicroC_WriteCmd(const struct PCommand::I2C_Parameters &i2c_p)
{
    union i2c_smbus_data data;
    uint8_t cmd = 0;

        // Moteur Gauche
            cmd =  (1 << 5)
                + (i2c_p.motorP.RAZdefaultGauche << 4)
                + (i2c_p.motorP.renvoieDistanceGauche << 3)
                + (1 << 2)
                + (i2c_p.motorP.directionGauche << 1)
                + (i2c_p.motorP.vitesseProgressiveGauche);

            data.byte = i2c_p.motorP.vitesseGauche;

            SetAdresse(0x10);
            BusAccess(I2C_SMBUS_WRITE,cmd,I2C_SMBUS_BYTE_DATA, &data);

        //Moteur Droit
            cmd = 0;
            cmd = (1 << 5)
                + (i2c_p.motorP.RAZdefaultDroite << 4)
                + (i2c_p.motorP.renvoieDistanceDroite << 3)
                + (1 << 2)
                + (i2c_p.motorP.directionDroite << 1)
                + (i2c_p.motorP.vitesseProgressiveDroite);

            data.byte = i2c_p.motorP.vitesseDroite;

            SetAdresse(0x20);
            BusAccess(I2C_SMBUS_WRITE,cmd,I2C_SMBUS_BYTE_DATA, &data);
}

void PI2C::MicroC_ShutdownMoteur()
{
    union i2c_smbus_data data;
    data.byte = 0;

// Moteur Gauche
    SetAdresse(0x10);
    BusAccess(I2C_SMBUS_WRITE,(1 << 5),I2C_SMBUS_BYTE_DATA, &data);

//Moteur Droit
    SetAdresse(0x20);
    BusAccess(I2C_SMBUS_WRITE,(1 << 5),I2C_SMBUS_BYTE_DATA, &data);
}
