#include "PRobot.hpp"

using namespace std;

PRobot::PRobot() : mPm(PGpioManager::getInstance())
{
    //ctor
}

PRobot::~PRobot()
{
    //dtor
}
void PRobot::preRun()
{
    bindCommandeQueue(Agent::Network, mNetwork.getCommandQueue());
    mNetwork.bindMaster(this);
    mNetwork.start();

    bindCommandeQueue(Agent::I2C, mI2C.getCommandQueue());
    mI2C.bindMaster(this);
    mI2C.start();

    bindCommandeQueue(Agent::US, mUS.getCommandQueue());
    mUS.bindMaster(this);
    mUS.start();
}

void PRobot::run()
{

}

void PRobot::postRun()
{
    mNetwork.stop();
    mI2C.stop();
    mUS.stop();
}

void PRobot::handleEvent(const PEvent& event)
{

    switch(event.mAgent)
    {
        case Agent::I2C:
        {
            handleI2CEvent(event);
            break;
        }
        case Agent::Network:
        {
            handleNetworkEvent(event);
            break;
        }
        case Agent::US:
        {
            handleUSEvent(event);
            break;
        }
    }
}

void PRobot::handleNetworkEvent(const PEvent &event)
{
    PCommand command;

    switch(event.network_p.type)
    {
        case PEvent::Network_Parameters::Network_Event::ClientConnected:
        {
            cout <<"Event ClientConnected !"<<endl;
            command.mAgent=Agent::I2C;
            command.i2c_p.type = PCommand::I2C_Parameters::I2C_Command::VerifDefaultMotor;
            pushCommand(command);

            command.mAgent = Agent::US;
            command.us_p.type = PCommand::US_Parameters::US_Command::StartAvant;
            pushCommand(command);
            break;
        }
        case PEvent::Network_Parameters::Network_Event::ClientDisconnected:
        {
            cout <<"Event ClientDisconnected !"<<endl;
            command.mAgent = Agent::I2C;
            command.i2c_p.type = PCommand::I2C_Parameters::I2C_Command::StopMoteur;
            pushCommand(command);

            command.mAgent = Agent::US;
            command.us_p.type = PCommand::US_Parameters::US_Command::StopUS;
            pushCommand(command);
            break;
        }
        case PEvent::Network_Parameters::Network_Event::Motion:
        {
            switch(event.network_p.motion_p.type)
            {
                case PEvent::Network_Parameters::MotionParameters::MotionType::Joystick:
                {
                    command = mCB_Moteur.updateWithJoystick(event.network_p.motion_p.joystick);
                    pushCommand(command);
                    if (command.i2c_p.motorP.directionDroite == true && command.i2c_p.motorP.directionGauche == true)
                    {
                        command.mAgent = Agent::US;
                        command.us_p.type = PCommand::US_Parameters::US_Command::StartAvant;
                        pushCommand(command);
                    }
                    else if (command.i2c_p.motorP.directionDroite == false && command.i2c_p.motorP.directionGauche == false)
                    {
                        command.mAgent = Agent::US;
                        command.us_p.type = PCommand::US_Parameters::US_Command::StartArriere;
                        pushCommand(command);
                    }
                    break;

                }
                case PEvent::Network_Parameters::MotionParameters::MotionType::Rotation:
                {
                    pushCommand(mCB_Moteur.updateWithRotation(event.network_p.motion_p.rotation_p));
                    command.mAgent = Agent::US;
                    command.us_p.type = PCommand::US_Parameters::US_Command::StopUS;
                    pushCommand(command);
                    break;
                }
            }
            break;
        }
        case PEvent::Network_Parameters::Network_Event::ButtonRAZDefaults :
        {
            command.mAgent = Agent::I2C;
            command.i2c_p.type = PCommand::I2C_Parameters::I2C_Command::RAZDefaultMotor;
            pushCommand(command);

            command.mAgent = Agent::US;
            command.us_p.type = PCommand::US_Parameters::US_Command::Reset;
            pushCommand(command);
            break;
        }
    }
}

void PRobot::handleI2CEvent(const PEvent &event)
{
    switch(event.i2c_p.type)
    {
        case PEvent::I2C_Parameters::I2C_Event::OpenFailed :
        {
            cout << "Le Peripherique I2C n'a pas pu etre ouvert" << endl ;
            break;
        }
        case PEvent::I2C_Parameters::I2C_Event::SetAddressFailed :
        {
            cout << mI2C.fromDeviceToString(event.i2c_p.device) << " : L'adresse de l'esclave n'a pas pu etre selectionee" <<endl;
            break;
        }
        case PEvent::I2C_Parameters::I2C_Event::WriteFailed :
        {
            cout << mI2C.fromDeviceToString(event.i2c_p.device) << " : L'ecriture a echoue" << endl;
            break;
        }
        case PEvent::I2C_Parameters::I2C_Event::I2C_ErrorBattery :
        {
            cout << mI2C.fromDeviceToString(event.i2c_p.device) << " : Erreur Batterie" << endl;
            break;
        }
        case PEvent::I2C_Parameters::I2C_Event::I2C_ErrorMotorDriver :
        {
            cout << mI2C.fromDeviceToString(event.i2c_p.device) << " : Erreur Driver Moteur" << endl;
            break;
        }
        case PEvent::I2C_Parameters::I2C_Event::I2C_ErrorTimeOut :
        {
            cout << mI2C.fromDeviceToString(event.i2c_p.device) << " : Erreur Time Out" << endl;
            break;
        }
        case PEvent::I2C_Parameters::I2C_Event::I2C_DistanceParcouru :
        {
            //cout << "Distance moteur gauche : " << +event.i2c_p.distanceGauche << endl;
            //cout << "Distance moteur droit : " << +event.i2c_p.distanceDroite << endl;
            break;
        }
        case PEvent::I2C_Parameters::I2C_Event::I2C_DistanceArret :
        {
            cout << "Distance d'arret moteur gauche : " << +event.i2c_p.distanceArretGauche << endl;
            cout << "Distance d'arret moteur droit : " << +event.i2c_p.distanceArretDroite << endl;
            break;
        }
    }
}

void PRobot::handleUSEvent(const PEvent &event)
{
    switch (event.us_p.type)
    {
        case PEvent::US_Parameters::US_Event::US_Obstacle :
        {
            if (event.us_p.device == PEvent::US_Parameters::US_Device::CapteurAvant)
                cout << "Capteur US Avant : Obstacle" << endl;
            else if (event.us_p.device == PEvent::US_Parameters::US_Device::CapteurArriere)
                cout << "Capteur US Arriere : Obstacle" << endl;

            pushCommand(mCB_Moteur.updateWithUS(event.us_p.seuil));


            break;
        }
        case PEvent::US_Parameters::US_Event::US_Error :
        {
            if (event.us_p.device == PEvent::US_Parameters::US_Device::CapteurAvant)
                cout << "Capteur US Avant : Erreur" << endl;
            else if (event.us_p.device == PEvent::US_Parameters::US_Device::CapteurArriere)
                cout << "Capteur US Arriere : Erreur" << endl;
        }
        case PEvent::US_Parameters::US_Event::US_Distance :
        {

            break;
        }
    }
}
