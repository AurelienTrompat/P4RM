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
    union i2c_smbus_data data;

    if (mNewCommand)
    {
        switch (mI2C_Command)
        {
            case i2c_Command::SetCommandMotor :
            {
                //Moteur Gauche
                SetAdresse(0x10);
                data.byte = mCmdMoteur.VitesseGauche;
                BusAccess(I2C_SMBUS_WRITE ,mCmdMoteur.CmdMGauche ,I2C_SMBUS_BYTE_DATA , &data);

                //Moteur Droit
                SetAdresse(0x20);
                data.byte = mCmdMoteur.VitesseDroite;
                BusAccess(I2C_SMBUS_WRITE,mCmdMoteur.CmdMDroit,I2C_SMBUS_BYTE_DATA, &data);
                break;
            }
            case i2c_Command::StopMoteur :
            {
                MicroC_ShutdownMoteur();
                break;
            }
            case i2c_Command::Giroscope :
            {

                break;
            }
            case i2c_Command::AxelEtMagn :
            {

                break;
            }
            case i2c_Command::Laser :
            {

                break;
            }
            default :
                break;
        }
        mNewCommand = false;
    }
}

void PI2C::postRun()
{
    MicroC_ShutdownMoteur();
    close(mFd);
}

void PI2C::handleCommand(const PCommand& command)
{
    if (command.mAgent == PCommand::Agent::I2C)
    {
        switch (command.i2c_p.type)
        {
            case i2c_Command::SetCommandMotor :
            {
                MicroC_WriteCmd(command.i2c_p);
                break;
            }
            case i2c_Command::StopMoteur :
            {
                mI2C_Command = i2c_Command::StopMoteur;
                break;
            }
            case i2c_Command::Giroscope :
            {

                break;
            }
            case i2c_Command::AxelEtMagn :
            {

                break;
            }
            case i2c_Command::Laser :
            {

                break;
            }
            default :
                break;
        }
        mNewCommand = true;
        while(!(mNewCommand & getRunState()));
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

int PI2C::BusAccess (bool rw, uint8_t command, int dataSize, union i2c_smbus_data *data)
{
    struct i2c_smbus_ioctl_data args ;
    uint8_t i=0;

    args.read_write = rw ;
    args.command    = command ;
    args.size       = dataSize ;
    args.data       = data ;

    while((ioctl (mFd, I2C_SMBUS, &args) == -1)&& (i<3))
    {
        i++;
    }
    if (i == 3)
    {
        SendEvent(PEvent::Type::I2C_WriteFailed);
        return -1;
    }
    else
    {
        return 0;
    }
}

void PI2C::MicroC_WriteCmd(const struct PCommand::I2C_Parameters &i2c_p)
{
    // Moteur Gauche
    mCmdMoteur.CmdMGauche   =  (1 << 5)
                            + (i2c_p.motorP.RAZdefaultGauche << 4)
                            + (i2c_p.motorP.renvoieDistanceGauche << 3)
                            + (1 << 2)
                            + (i2c_p.motorP.directionGauche << 1)
                            + (i2c_p.motorP.vitesseProgressiveGauche);

    mCmdMoteur.VitesseGauche = i2c_p.motorP.vitesseGauche;

    //Moteur Droit
    mCmdMoteur.CmdMDroit    = (1 << 5)
                            + (i2c_p.motorP.RAZdefaultDroite << 4)
                            + (i2c_p.motorP.renvoieDistanceDroite << 3)
                            + (1 << 2)
                            + (i2c_p.motorP.directionDroite << 1)
                            + (i2c_p.motorP.vitesseProgressiveDroite);

    mCmdMoteur.VitesseDroite = i2c_p.motorP.vitesseDroite;
    mI2C_Command = i2c_Command::SetCommandMotor;

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
