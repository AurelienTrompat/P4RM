#include "PRobot.hpp"

using namespace std;

PRobot::PRobot() : mPm(PGpioManager::getInstance())
{
    mDG=0;
    mDD=0;
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

    bindCommandeQueue(Agent::PositionTracker, mPosTracker.getCommandQueue());
    mPosTracker.bindMaster(this);
    mPosTracker.start();
}

void PRobot::run()
{

}

void PRobot::postRun()
{
    mNetwork.stop();
    mI2C.stop();
    mUS.stop();
    mPosTracker.stop();
    cout << "Stop PRobot" <<endl;
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
        case Agent::PositionTracker:
        {
            handlePosTrackerEvent(event);
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
            command.i2c_p.type = PCommand::I2C_Parameters::I2C_Command::MicroC_VerifDefaultMotor;
            pushCommand(command);
            command.i2c_p.type = PCommand::I2C_Parameters::I2C_Command::Gyro_Start;
            pushCommand(command);
            command.i2c_p.type = PCommand::I2C_Parameters::I2C_Command::Magn_Start;
            pushCommand(command);
            command.i2c_p.type = PCommand::I2C_Parameters::I2C_Command::Axel_Start;
            pushCommand(command);
            break;
        }
        case PEvent::Network_Parameters::Network_Event::ClientDisconnected:
        {
            cout <<"Event ClientDisconnected !"<<endl;
            command.mAgent = Agent::I2C;
            command.i2c_p.type = PCommand::I2C_Parameters::I2C_Command::MicroC_StopMoteur;
            pushCommand(command);
            command.i2c_p.type = PCommand::I2C_Parameters::I2C_Command::MicroC_VerifDefaultMotor;
            pushCommand(command);
            command.i2c_p.type = PCommand::I2C_Parameters::I2C_Command::Gyro_Stop;
            pushCommand(command);
            command.i2c_p.type = PCommand::I2C_Parameters::I2C_Command::Magn_Stop;
            pushCommand(command);
            command.i2c_p.type = PCommand::I2C_Parameters::I2C_Command::Axel_Stop;
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
                    uint8_t usUpdate = mCB_Moteur.getEtatUS();
                    command.mAgent = Agent::US;
                        switch (usUpdate)
                        {
                            case 0:
                            {
                                command.us_p.type = PCommand::US_Parameters::US_Command::StopUS;
                                break;
                            }
                            case 1:
                            {
                                command.us_p.type = PCommand::US_Parameters::US_Command::StartAvant;
                                break;
                            }
                            case 2:
                            {
                                command.us_p.type = PCommand::US_Parameters::US_Command::StartArriere;
                                break;
                            }
                            default :
                                break;
                        }
                        if (usUpdate !=3)
                            pushCommand(command);
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
            command.i2c_p.type = PCommand::I2C_Parameters::I2C_Command::MicroC_RAZDefaultMotor;
            pushCommand(command);

            command.mAgent = Agent::US;
            command.us_p.type = PCommand::US_Parameters::US_Command::Reset;
            pushCommand(command);
            break;
        }
        case PEvent::Network_Parameters::Network_Event::ButtonRAZPosition :
        {
            command.mAgent = Agent::PositionTracker;
            command.posTracker_p.type = PCommand::PositionTracker_Parameters::PositionTracker_Command::ResetPosition;
            pushCommand(command);
            break;
        }
    }
}

void PRobot::handleI2CEvent(const PEvent &event)
{
    PCommand command;
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
            //cout << "Distance moteur gauche : " << +event.i2c_p.distanceGauche/*fromStepToCentimeter(event.i2c_p.distanceGauche)*/ << endl;
            //cout << "Distance moteur droit : " << +event.i2c_p.distanceDroite/*fromStepToCentimeter(event.i2c_p.distanceDroite)*/ << endl;
            convertAndRelayDist(event.i2c_p.distanceGauche, event.i2c_p.distanceDroite);
            mDG+=event.i2c_p.distanceGauche;
            mDD+=event.i2c_p.distanceDroite;
            break;
        }
        case PEvent::I2C_Parameters::I2C_Event::I2C_DistanceArret :
        {
            //cout << "Distance d'arret moteur gauche : " << +event.i2c_p.distanceArretGauche/*fromStepToCentimeter(event.i2c_p.distanceArretGauche)*/ << endl;
            //cout << "Distance d'arret moteur droit : " << +event.i2c_p.distanceArretDroite/*fromStepToCentimeter(event.i2c_p.distanceArretDroite)*/ << endl;
            convertAndRelayDist(event.i2c_p.distanceArretGauche, event.i2c_p.distanceArretDroite);
            mDG+=event.i2c_p.distanceArretGauche;
            mDD+=event.i2c_p.distanceArretDroite;
            break;
        }
        case PEvent::I2C_Parameters::I2C_Event::I2C_ZAxisAngularData :
        {
            //cout << mI2C.fromDeviceToString(event.i2c_p.device) << "rotation relative du robot = " << +event.i2c_p.angularData << endl;
            //cout << mI2C.fromDeviceToString(event.i2c_p.device) << "rotation du robot par rapport au nord = " << +event.i2c_p.angularData << endl;
            command.mAgent=Agent::PositionTracker;
            command.posTracker_p.type = PCommand::PositionTracker_Parameters::PositionTracker_Command::UpdateAngle;
            command.posTracker_p.angleGyro=event.i2c_p.angularData;
            pushCommand(command);
            break;

        }
        case PEvent::I2C_Parameters::I2C_Event::I2C_ErrorRobotLift :
        {
            cout << mI2C.fromDeviceToString(event.i2c_p.device) << " : Erreur !!! Repose ce robot a terre" << endl;
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
            /*if (event.us_p.device == PEvent::US_Parameters::US_Device::CapteurAvant)
                cout << "Capteur US Avant : Obstacle" << endl;
            else if (event.us_p.device == PEvent::US_Parameters::US_Device::CapteurArriere)
                cout << "Capteur US Arriere : Obstacle" << endl;*/

            pushCommand(mCB_Moteur.updateWithUS(event.us_p.seuil));
            break;
        }
        case PEvent::US_Parameters::US_Event::US_Error :
        {
            if (event.us_p.device == PEvent::US_Parameters::US_Device::CapteurAvant)
                cout << "Capteur US Avant : Erreur" << endl;
            else if (event.us_p.device == PEvent::US_Parameters::US_Device::CapteurArriere)
                cout << "Capteur US Arriere : Erreur" << endl;
            break;
        }
        case PEvent::US_Parameters::US_Event::US_Distance :
        {
            break;
        }
    }
}
void PRobot::handlePosTrackerEvent(const PEvent& event)
{
    PCommand command;
    switch(event.posTracker_p.type)
    {
        case PEvent::PositionTracker_Parameters::PositionTracker_Event::PosHasChanged:
        {
            //cout << mDG << "\t" << mDD <<endl;
            //cout << "x : " << +event.posTracker_p.pos.x/*fromStepToCentimeter(event.posTracker_p.pos.x)*/ <<endl;
            //cout << "y : " << +event.posTracker_p.pos.y/*fromStepToCentimeter(event.posTracker_p.pos.y)*/ <<endl;
            //cout << "phi : " << +event.posTracker_p.pos.phi*2*M_PI/360 <<endl;

            command.mAgent = Agent::Network;
            command.network_p.type = PCommand::Network_Parameters::Network_Command::NewPosition;
            command.network_p.pos.x = fromStepToCentimeter(event.posTracker_p.pos.x);
            command.network_p.pos.y = fromStepToCentimeter(event.posTracker_p.pos.y);
            command.network_p.pos.phi = event.posTracker_p.pos.phi*2*M_PI/360;
            pushCommand(command);
        }
    }
}
void PRobot::convertAndRelayDist(uint16_t leftDist, uint16_t rightDist)
{
    PCommand command;
    command.mAgent = Agent::PositionTracker;
    command.posTracker_p.type = PCommand::PositionTracker_Parameters::PositionTracker_Command::UpdatePosition;

    if(mCB_Moteur.getLeftDirection())
        command.posTracker_p.traveledDist.leftDist=(int16_t)leftDist;
    else
        command.posTracker_p.traveledDist.leftDist=-(int16_t)leftDist;

    if(mCB_Moteur.getRightDirection())
        command.posTracker_p.traveledDist.rightDist=(int16_t)rightDist;
    else
        command.posTracker_p.traveledDist.rightDist=-(int16_t)rightDist;

    pushCommand(command);
}
int16_t PRobot::fromStepToCentimeter(int16_t dist)
{
    return dist*((12*M_PI)/1000);
}
