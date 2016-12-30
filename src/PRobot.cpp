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

    PCommand command;
    command.mType = PCommand::Type::Test;
    pushCommand(command);
}

void PRobot::run()
{

}

void PRobot::postRun()
{
    mNetwork.stop();
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
        default:
        {}
    }
}
