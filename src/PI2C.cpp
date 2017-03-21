#include "PI2C.hpp"
#include"PMicro_C.hpp"

using namespace std;

PI2C::PI2C() : mMicroC(this), mModule9DOF(this), mNewCommand(false)
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
    this_thread::sleep_for(chrono::milliseconds(1));
    mModule9DOF.Gyro_Init();
    mModule9DOF.Gyro_Start();
}

void PI2C::run()
{

    mMicroC.MicroC_ActiverRenvoieDistance();

    mMicroC.MicroC_DemmanderDistance();

    mMicroC.MicroC_Ping();

    mModule9DOF.Gyro_CheckAngle();

    if (mNewCommand)
    {
        switch (mI2C_Command)
        {
            case i2c_Command::SetCommandMotor :
            {
                mMicroC.MicroC_SetCommandMoteur();
                break;
            }
            case i2c_Command::StopMoteur :
            {
                mMicroC.MicroC_ShutdownMoteur();
                break;
            }
            case i2c_Command::RAZDefaultMotor :
            {
                mMicroC.MicroC_RAZDefault();
                mModule9DOF.Gyro_RAZDefault();
            }
            case i2c_Command::VerifDefaultMotor :
            {
                mMicroC.MicroC_VerifDefault();
            }
            case i2c_Command::Gyro :
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
    mMicroC.MicroC_ShutdownMoteur();
    mModule9DOF.Gyro_Shutdown();
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
                mMicroC.MicroC_WriteCmd(command.i2c_p);
                break;
            }
            case i2c_Command::StopMoteur :
            case i2c_Command::RAZDefaultMotor :
            case i2c_Command::VerifDefaultMotor :
            {
                mI2C_Command = command.i2c_p.type;
                break;
            }
            case i2c_Command::Gyro :
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

void PI2C::SendEvent(i2c_Event typeEvent, int16_t angularData)
{
    PEvent event;

    event.i2c_p.type = typeEvent;
    event.i2c_p.angularData = angularData;

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

    while((ioctl (mFd, I2C_SMBUS, &args) == -1)&& (i<10))
    {
        i++;
        this_thread::sleep_for(chrono::microseconds(100));
    }
    if (i == 10)
    {
        SendEvent(i2c_Event::WriteFailed);
        return -1;
    }
    else
    {
        return 0;
    }
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
        case i2c_Device::Gyro :
            return "Gyroscope";
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
