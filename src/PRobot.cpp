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
    uint16_t cg, cd;
    PCommand command;
    static bool dir;
    switch(event.mType)
    {
        case PEvent::Type::Joystick:
        {
            //cout <<"Event Joystick !!! ("<<+event.joystick.x<<", "<<+event.joystick.y<<")"<<endl;

            command.mAgent=PCommand::Agent::I2C;
            command.i2c_p.type =PCommand::I2C_Parameters::I2C_Command::SetCommandMotor;

            command.i2c_p.motorP.RAZdefaultDroite = false;
            command.i2c_p.motorP.RAZdefaultGauche = false;
            command.i2c_p.motorP.renvoieDistanceDroite = false;
            command.i2c_p.motorP.renvoieDistanceGauche = false;
            command.i2c_p.motorP.vitesseProgressiveDroite = false;
            command.i2c_p.motorP.vitesseProgressiveGauche = false;

            if(event.joystick.y > 128)
            {
                cg=((event.joystick.y)-128)*2;
                command.i2c_p.motorP.directionDroite = true;
                command.i2c_p.motorP.directionGauche = true;
                dir=true;
            }
            else if(event.joystick.y < 128)
            {
                cg=((128-event.joystick.y))*2;
                command.i2c_p.motorP.directionDroite = false;
                command.i2c_p.motorP.directionGauche = false;
                dir=false;
            }
            else
            {
                command.i2c_p.motorP.directionDroite = dir;
                command.i2c_p.motorP.directionGauche = dir;
                //cout << dir<<endl;
                cg=0;
            }


            cd=cg;

            if(event.joystick.x < 128)
            {
                if(cg>=128-event.joystick.x)
                    cg-=128-event.joystick.x;
                else
                    cg=0;

                cd+=128-event.joystick.x;
            }
            else if(event.joystick.x > 128)
            {
                if(cd>=(event.joystick.x-128))
                    cd-=(event.joystick.x-128);
                else
                    cd=0;
                cg+=(event.joystick.x-128);
            }

            if(cg > 255)
                cg=255;
            if(cd > 255)
                cd=255;


           // cout<<+((uint8_t)cg) <<" "<<+((uint8_t)cd)<<endl;
            command.i2c_p.motorP.vitesseDroite = (uint8_t)cd;
            command.i2c_p.motorP.vitesseGauche = (uint8_t)cg;

            pushCommand(command);
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
            command.mAgent = PCommand::Agent::I2C;
            command.i2c_p.type = PCommand::I2C_Parameters::I2C_Command::StopMoteur;
            pushCommand(command);
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
        case PEvent::Type::I2C_WriteFailed :
        {
            cout << "L'ecriture a echoue" << endl;
            break;
        }

        default:
        {}
    }
}
