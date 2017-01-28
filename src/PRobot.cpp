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



}

void PRobot::run()
{

}

void PRobot::postRun()
{
    mNetwork.stop();
    mI2C.stop();
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
    }
}

void PRobot::handleNetworkEvent(const PEvent &event)
{
    uint16_t cg, cd;
    static bool dir;

    PCommand command;

    switch(event.network_p.type)
    {
        case PEvent::Network_Parameters::Network_Event::ClientConnected:
        {
            cout <<"Event ClientConnected !"<<endl;
            command.mAgent=Agent::I2C;
            command.i2c_p.type = PCommand::I2C_Parameters::I2C_Command::VerifDefaultMotor;
            pushCommand(command);
            break;
        }
        case PEvent::Network_Parameters::Network_Event::ClientDisconnected:
        {
            cout <<"Event ClientDisconnected !"<<endl;
            command.mAgent = Agent::I2C;
            command.i2c_p.type = PCommand::I2C_Parameters::I2C_Command::StopMoteur;
            pushCommand(command);
            break;
        }
        case PEvent::Network_Parameters::Network_Event::JoystickMoved:
        {
            const uint8_t &x = event.network_p.joystick.x;
            const uint8_t &y = event.network_p.joystick.y;

            command.mAgent=Agent::I2C;
            command.i2c_p.type = PCommand::I2C_Parameters::I2C_Command::SetCommandMotor;

            command.i2c_p.motorP.RAZdefaultDroite = false;
            command.i2c_p.motorP.RAZdefaultGauche = false;
            command.i2c_p.motorP.renvoieDistance = true;
            command.i2c_p.motorP.vitesseProgressiveDroite =  true;
            command.i2c_p.motorP.vitesseProgressiveGauche = true;

            if(y > 128)
            {
                cg=(y-128)*2;
                command.i2c_p.motorP.directionDroite = true;
                command.i2c_p.motorP.directionGauche = true;
                dir=true;
            }
            else if(y < 128)
            {
                cg=((128-y))*2;
                command.i2c_p.motorP.directionDroite = false;
                command.i2c_p.motorP.directionGauche = false;
                dir=false;
            }
            else
            {
                command.i2c_p.motorP.directionDroite = dir;
                command.i2c_p.motorP.directionGauche = dir;
                cg=0;
            }


            cd=cg;

            if(x < 128)
            {
                if(cg>=128-x)
                    cg-=128-x;
                else
                    cg=0;

                cd+=128-x;
            }
            else if(x > 128)
            {
                if(cd>=(x-128))
                    cd-=(x-128);
                else
                    cd=0;
                cg+=(x-128);
            }

            if(cg > 255)
                cg=255;
            if(cd > 255)
                cd=255;

            command.i2c_p.motorP.vitesseDroite = (uint8_t)cd;
            command.i2c_p.motorP.vitesseGauche = (uint8_t)cg;

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
