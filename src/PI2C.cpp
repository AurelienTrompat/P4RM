#include "PI2C.hpp"

using namespace std;

PI2C::PI2C() : mMoteurGErreur(false), mMoteurDErreur(false), mArretGauche(0), mArretDroit(0), mNewCommand(false), mActiverRenvoieDist(false), mRenvoieDistance(false)
{
   setAgent(Agent::I2C);
}

PI2C::~PI2C()
{
    //dtor
}

void PI2C::preRun()
{
    mI2C_Device = i2c_Device::I2C;
    OpenI2C();
}

void PI2C::run()
{
    union i2c_smbus_data data;

    if (mActiverRenvoieDist)
    {
        mRenvoieDistance = true;
        mChronoDistance = Chrono::now();
        mActiverRenvoieDist = false;
    }

    std::chrono::duration<double> diff = Chrono::now() - mChronoDistance;
    if ((diff.count() > 0.05))
    {
        mChronoDistance = Chrono::now();
        if (mRenvoieDistance)
        {
            if (!mMoteurGErreur && !mMoteurDErreur)
                MicroC_ReadErreurEtVitesse();
            else
                MicroC_DistanceArret();
        }
        else
        {
            MicroC_VerifDefault();
        }
    }



    diff = Chrono::now() - mChronoPing;
    if (diff.count() > 0.1)
    {
        mChronoPing = Chrono::now();
        if (((mCmdMoteur.CmdMGauche & (1 << 2)) == (1 << 2)) && ((mCmdMoteur.CmdMGauche & (1 << 2)) == (1 << 2)) && (!mMoteurGErreur && !mMoteurDErreur))
        {
                //Moteur Gauche
                data.byte = mCmdMoteur.VitesseGauche;
                SetAdresse(0x10);
                mI2C_Device = i2c_Device::MoteurGauche;
                BusAccess(I2C_SMBUS_WRITE ,mCmdMoteur.CmdMGauche ,I2C_SMBUS_BYTE_DATA , &data);

                //Moteur Droit
                data.byte = mCmdMoteur.VitesseDroite;
                mI2C_Device = i2c_Device::MoteurDroit;
                SetAdresse(0x20);
                BusAccess(I2C_SMBUS_WRITE,mCmdMoteur.CmdMDroit,I2C_SMBUS_BYTE_DATA, &data);
        }
        else
        {
            MicroC_ShutdownMoteur();
        }
    }


    if (mNewCommand)
    {
        switch (mI2C_Command)
        {
            case i2c_Command::SetCommandMotor :
            {
                if (!mMoteurGErreur && !mMoteurDErreur)
                {
                    //Moteur Gauche
                    data.byte = mCmdMoteur.VitesseGauche;
                    SetAdresse(0x10);
                    mI2C_Device = i2c_Device::MoteurGauche;
                    BusAccess(I2C_SMBUS_WRITE ,mCmdMoteur.CmdMGauche ,I2C_SMBUS_BYTE_DATA , &data);

                    //Moteur Droit
                    data.byte = mCmdMoteur.VitesseDroite;
                    mI2C_Device = i2c_Device::MoteurDroit;
                    SetAdresse(0x20);
                    BusAccess(I2C_SMBUS_WRITE,mCmdMoteur.CmdMDroit,I2C_SMBUS_BYTE_DATA, &data);
                }
                break;
            }
            case i2c_Command::StopMoteur :
            {
                MicroC_ShutdownMoteur();
                mRenvoieDistance = false;
                break;
            }
            case i2c_Command::RAZDefaultMotor :
            {
                MicroC_RAZDefault();
                mMoteurGErreur = false;
                mMoteurDErreur = false;
            }
            case i2c_Command::VerifDefaultMotor :
            {
                MicroC_VerifDefault();
            }
            case i2c_Command::Giro :
            {

                break;
            }
            case i2c_Command::Axel :
            {

                break;
            }
            case i2c_Command::Magn :
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
    cout << "i2cStop" <<endl;
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
            case i2c_Command::RAZDefaultMotor :
            case i2c_Command::VerifDefaultMotor :
            {
                mI2C_Command = command.i2c_p.type;
                break;
            }
            case i2c_Command::Giro :
            {

                break;
            }
            case i2c_Command::Axel :
            {

                break;
            }
            case i2c_Command::Magn :
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

void PI2C::SendEvent(i2c_Event typeEvent)
{
    PEvent event;
    event.i2c_p.type = typeEvent;

        event.i2c_p.device = mI2C_Device;
    pushEvent(event);
}

void PI2C::SendEvent(i2c_Event typeEvent, uint16_t distanceGauche, uint16_t distanceDroite)
{
    PEvent event;

    event.i2c_p.type = typeEvent;

    if(typeEvent == i2c_Event::I2C_DistanceParcouru)
    {
        event.i2c_p.distanceDroite = distanceDroite;
        event.i2c_p.distanceGauche = distanceGauche;
    }
    if(typeEvent == i2c_Event::I2C_DistanceArret)
    {
        event.i2c_p.distanceArretDroite = distanceDroite;
        event.i2c_p.distanceArretGauche = distanceGauche;
    }

    pushEvent(event);
}

void PI2C::OpenI2C()
{

    if ((mFd = open("/dev/i2c-1",O_RDWR))== -1)
    {
        SendEvent(i2c_Event::OpenFailed);
    }

}
void PI2C::SetAdresse(uint8_t adresse)
{

    if (ioctl(mFd, I2C_SLAVE, adresse)== -1)
    {
        SendEvent(i2c_Event::SetAddressFailed);
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
    this_thread::sleep_for(chrono::microseconds(100));
    if (i == 3)
    {
        SendEvent(i2c_Event::WriteFailed);
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
                            + (0 << 4)
                            + (i2c_p.motorP.renvoieDistance << 3)
                            + (1 << 2)
                            + (i2c_p.motorP.directionGauche << 1)
                            + (i2c_p.motorP.vitesseProgressiveGauche);

    mCmdMoteur.VitesseGauche = i2c_p.motorP.vitesseGauche;

    //Moteur Droit
    mCmdMoteur.CmdMDroit    = (1 << 5)
                            + (0 << 4)
                            + (i2c_p.motorP.renvoieDistance << 3)
                            + (1 << 2)
                            + (i2c_p.motorP.directionDroite << 1)
                            + (i2c_p.motorP.vitesseProgressiveDroite);

    mCmdMoteur.VitesseDroite = i2c_p.motorP.vitesseDroite;
    mI2C_Command = i2c_Command::SetCommandMotor;

    if (i2c_p.motorP.renvoieDistance && !mRenvoieDistance && !mMoteurGErreur && !mMoteurDErreur)
    {
        mActiverRenvoieDist = true;
    }

}

void PI2C::MicroC_ShutdownMoteur()
{
    union i2c_smbus_data data;
    data.byte = 0;

// Moteur Gauche
    mI2C_Device = i2c_Device::MoteurGauche;
    SetAdresse(0x10);
    BusAccess(I2C_SMBUS_WRITE,(1 << 5),I2C_SMBUS_BYTE_DATA, &data);

//Moteur Droit
    mI2C_Device = i2c_Device::MoteurDroit;
    SetAdresse(0x20);
    BusAccess(I2C_SMBUS_WRITE,(1 << 5),I2C_SMBUS_BYTE_DATA, &data);
}

void PI2C::MicroC_ReadErreurEtVitesse()
{
    union i2c_smbus_data data;

    mI2C_Device = i2c_Device::MoteurGauche;
    SetAdresse(0x10);
    int lectureGauche = BusAccess(I2C_SMBUS_READ, 0x00, I2C_SMBUS_WORD_DATA, &data);
    uint16_t distanceGauche = ((data.word & 0x7f00) >> 8) + ((data.word & 0x0040) << 1);
    int erreurMoteurGauche = MicroC_CheckError((uint8_t)(data.word & 0x0007));
    if (erreurMoteurGauche == -1)
        mMoteurGErreur = true;
    else
        mMoteurGErreur = false;

    mI2C_Device = i2c_Device::MoteurDroit;
    SetAdresse(0x20);
    int lectureDroite = BusAccess(I2C_SMBUS_READ, 0x00, I2C_SMBUS_WORD_DATA, &data);
    uint16_t distanceDroit = ((data.word & 0x7f00) >> 8) + ((data.word & 0x0040) << 1);
    int erreurMoteurDroit = MicroC_CheckError((uint8_t)(data.word & 0x0007));
    if(erreurMoteurDroit == -1)
        mMoteurDErreur = true;
    else
        mMoteurDErreur = false;

    if (lectureGauche != -1 && lectureDroite != -1 && erreurMoteurGauche == 0 && erreurMoteurDroit == 0)
    {
        SendEvent(i2c_Event::I2C_DistanceParcouru, distanceGauche, distanceDroit);
    }
}

int PI2C::MicroC_CheckError(uint8_t error)
{
    if (error != 0)
    {
        if(!mMoteurGErreur && !mMoteurDErreur)
        {
           if((error & 0x02) == 0x02)
                SendEvent(i2c_Event::I2C_ErrorBattery);
            if((error & 0x04) == 0x04)
                SendEvent(i2c_Event::I2C_ErrorMotorDriver);
            if((error & 0x01) == 0x01)
                SendEvent(i2c_Event::I2C_ErrorTimeOut);
            MicroC_ShutdownMoteur();
        }
        return -1;
    }
    else
    {
        return 0;
    }
}

void PI2C::MicroC_DistanceArret()
{
    union i2c_smbus_data data;

//Moteur Gauche
    mI2C_Device = i2c_Device::MoteurGauche;
    SetAdresse(0x10);
    int lectureGauche = BusAccess(I2C_SMBUS_READ, 0x00, I2C_SMBUS_WORD_DATA, &data);
    uint16_t distanceArretGauche = 255*((data.word & 0x0038) >> 3) + ((data.word & 0x7f00) >> 8) + ((data.word & 0x0040) << 1);

//Moteur Droit
    mI2C_Device = i2c_Device::MoteurDroit;
    SetAdresse(0x20);
    int lectureDroit = BusAccess(I2C_SMBUS_READ, 0x00, I2C_SMBUS_WORD_DATA, &data);
    uint16_t distanceArretDroit = 255*((data.word & 0x0038) >> 3) + ((data.word & 0x7f00) >> 8) + ((data.word & 0x0040) << 1);

    if (lectureGauche != -1 && lectureDroit != -1 && distanceArretGauche == mArretGauche && distanceArretDroit == mArretDroit)
    {
        SendEvent(i2c_Event::I2C_DistanceArret,distanceArretGauche,distanceArretDroit);
        mRenvoieDistance=false;
    }
    mArretGauche = distanceArretGauche;
    mArretDroit = distanceArretDroit;
}

std::string PI2C::fromDeviceToString(const i2c_Device device)
{
    switch (device)
    {
        case i2c_Device::I2C :
            return "I2C";
        case i2c_Device::MoteurGauche :
            return "MoteurGauche";
        case i2c_Device::MoteurDroit :
            return "Moteur Droit";
        case i2c_Device::Giro :
            return "Giroscope";
        case i2c_Device::Axel :
            return "Axelerometre";
        case i2c_Device::Magn :
            return "Magnetometre";
        case i2c_Device::LaserAvant :
            return "Laser Avant";
        case i2c_Device::LaserGauche :
            return "Laser Gauche";
        case i2c_Device::LaserDroit :
            return "Laser Droit";
    }
    return "";
}

void PI2C::MicroC_RAZDefault()
{
  union i2c_smbus_data data;
    data.byte = 0;

// Moteur Gauche
    mI2C_Device = i2c_Device::MoteurGauche;
    SetAdresse(0x10);
    BusAccess(I2C_SMBUS_WRITE,((1 << 5)+(1 << 4)),I2C_SMBUS_BYTE_DATA, &data);

//Moteur Droit
    mI2C_Device = i2c_Device::MoteurDroit;
    SetAdresse(0x20);
    BusAccess(I2C_SMBUS_WRITE,((1 << 5)+(1 << 4)),I2C_SMBUS_BYTE_DATA, &data);
}
void PI2C::MicroC_VerifDefault()
{
    union i2c_smbus_data data;

     //Moteur Gauche
     SetAdresse(0x10);
     mI2C_Device = i2c_Device::MoteurGauche;
     if(BusAccess(I2C_SMBUS_READ ,0 ,I2C_SMBUS_BYTE, &data) != -1)
     {
        if(MicroC_CheckError((uint8_t)(data.byte & 0x0007))==-1)
            mMoteurGErreur = true;
        else
            mMoteurGErreur = false;
    }
    //Moteur Droit
    mI2C_Device = i2c_Device::MoteurDroit;
    SetAdresse(0x20);
    if(BusAccess(I2C_SMBUS_READ ,0 ,I2C_SMBUS_BYTE, &data) != -1)
    {
        if(MicroC_CheckError((uint8_t)(data.byte & 0x07))==-1)
            mMoteurDErreur = true;
        else
            mMoteurDErreur = false;
    }
}
