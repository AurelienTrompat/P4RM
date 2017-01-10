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
            cout << "Le Peripherique I2C est ouvert" << endl ;
            break;
        }
        case PEvent::Type::I2C_NotOpen :
        {
            cout << "Le Peripherique I2C n'a pas pu etre ouvert" << endl ;
            break;
        }
        case PEvent::Type::I2C_SetAdresseFailed :
        {
            cout <<"L'adresse de l'esclave n'a pas pu etre selectionee" <<endl;
            break;
        }
        case PEvent::Type::I2C_SetAdresseSuccess :
        {
            cout <<"L'adresse de l'esclave a ete selectionee" <<endl;
            break;
        }
        case PEvent::Type::I2C_WriteFailed :
        {
            cout << "L'ecriture a echoue" << endl;
            break;
        }
        case PEvent::Type::I2C_WriteSucess :
        {
            cout << "L'ecriture a reussi" << endl;
            break;
        }
        default:
        {}
    }
}
