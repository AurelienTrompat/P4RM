#include "PI2C.hpp"

using namespace std;

PI2C::PI2C() : mNewCommand(false), mActiverRenvoieDist(false), mRenvoieDistance(false)
{
   setAgent(Agent::I2C);
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

    if (mActiverRenvoieDist)
    {
        mRenvoieDistance = true;
        mChrono = Chrono::now();
        mActiverRenvoieDist = false;
    }

    std::chrono::duration<double> diff = Chrono::now() - mChrono;
    if ((diff.count() > 0.05) && mRenvoieDistance)
    {
        mChrono = Chrono::now();
        //MicroC_ReadErreurEtVitesse();
    }

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
    if (command.mAgent == Agent::I2C)
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

void PI2C::SendEvent(PEvent::I2C_Parameters::I2C_Event typeEvent)
{
    PEvent event;
    event.mAgent=Agent::I2C;
    event.i2c_p.type = typeEvent;
    pushEvent(event);
}

void PI2C::OpenI2C()
{

    if ((mFd = open("/dev/i2c-1",O_RDWR))== -1)
    {
        SendEvent(PEvent::I2C_Parameters::I2C_Event::OpenFailed);
    }

}
void PI2C::SetAdresse(uint8_t adresse)
{
    if (ioctl(mFd, I2C_SLAVE, adresse)== -1)
    {
        SendEvent(PEvent::I2C_Parameters::I2C_Event::SetAddressFailed);
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
    this_thread::sleep_for(chrono::microseconds(20));
    if (i == 3)
    {
        SendEvent(PEvent::I2C_Parameters::I2C_Event::WriteFailed);
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
                            + (i2c_p.motorP.renvoieDistance << 3)
                            + (1 << 2)
                            + (i2c_p.motorP.directionGauche << 1)
                            + (i2c_p.motorP.vitesseProgressiveGauche);

    mCmdMoteur.VitesseGauche = i2c_p.motorP.vitesseGauche;

    //Moteur Droit
    mCmdMoteur.CmdMDroit    = (1 << 5)
                            + (i2c_p.motorP.RAZdefaultDroite << 4)
                            + (i2c_p.motorP.renvoieDistance << 3)
                            + (1 << 2)
                            + (i2c_p.motorP.directionDroite << 1)
                            + (i2c_p.motorP.vitesseProgressiveDroite);

    mCmdMoteur.VitesseDroite = i2c_p.motorP.vitesseDroite;
    mI2C_Command = i2c_Command::SetCommandMotor;

    if (i2c_p.motorP.renvoieDistance && !mRenvoieDistance)
    {
        mActiverRenvoieDist = true;
    }

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

void PI2C::MicroC_ReadErreurEtVitesse()
{
   union i2c_smbus_data data;
   SetAdresse(0x10);
   if (BusAccess(I2C_SMBUS_READ, 0x00, I2C_SMBUS_WORD_DATA, &data))
        cout << -1 << endl;
   else
        cout << "gauche : " << ((data.word & 0xFF00)>>8) << endl;

    SetAdresse(0x20);
   if (BusAccess(I2C_SMBUS_READ, 0x00, I2C_SMBUS_WORD_DATA, &data))
        cout << -1 << endl;
   else
        cout << "droite : " << ((data.word & 0xFF00)>>8) << endl;
}
