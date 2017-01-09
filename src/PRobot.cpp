#include "PRobot.hpp"

using namespace std;

PRobot::PRobot()
{
    //ctor
}

PRobot::~PRobot()
{
    //dtor
}
void PRobot::preRun()
{
    bindCommandeQueue(PCommand::Agent::Network, mNetwork.getCommandQueue());
    mNetwork.bindMaster(this);
    mNetwork.start();

    bindCommandeQueue(PCommand::Agent::I2C, mI2C.getCommandQueue());
    mI2C.bindMaster(this);
    mI2C.start();

    PCommand command;
    command.mAgent = PCommand::Agent::I2C;
    command.mType = PCommand::Type::Test;
    pushCommand(command);
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
    switch(event.mType)
    {
        case PEvent::Type::Joystick:
        {
            cout <<"Event Joystick !!! ("<<+event.joystick.x<<", "<<+event.joystick.y<<")"<<endl;
            break;
        }
        case PEvent::Type::ClientConnected:
        {
            cout <<"Event ClientConnected !"<<endl;
            break;
        }
        case PEvent::Type::ClientDisconnected:
        {
            cout <<"Event ClientDisconnected !"<<endl;
            break;
        }
        case PEvent::Type::I2C_Open :
        {
            cout << "Le P�riph�rique I2C est ouvert" << endl ;
            break;
        }
        case PEvent::Type::I2C_NotOpen :
        {
            cout << "Le P�riph�rique I2C n'a pas pu �tre ouvert" << endl ;
            break;
        }
        default:
        {}
    }
}
