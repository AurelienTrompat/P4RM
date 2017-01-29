#include "PCB_Moteur.hpp"

PCB_Moteur::PCB_Moteur()
{
    mCommand.mAgent = Agent::I2C;
    mCommand.i2c_p.type = PCommand::I2C_Parameters::I2C_Command::SetCommandMotor;

    mCommand.i2c_p.motorP.renvoieDistance = false;
}

PCommand PCB_Moteur::updateWithJoystick(struct PEvent::Network_Parameters::JoystickParameters joystickData)
{
    const uint8_t &x = joystickData.x;
    const uint8_t &y = joystickData.y;

    PCommand::I2C_Parameters::MotorParameters &motorP = mCommand.i2c_p.motorP;

    uint8_t &cg = motorP.vitesseGauche;
    uint8_t &cd = motorP.vitesseDroite;

    motorP.vitesseProgressiveDroite =  true;
    motorP.vitesseProgressiveGauche = true;

    if(y > 128)
    {
        cg=(y-128)*2;
        motorP.directionDroite = true;
        motorP.directionGauche = true;
    }
    else if(y < 128)
    {
        if(y!=0)
            cg=((128-y))*2;
        else
            cg=255;
        motorP.directionDroite = false;
        motorP.directionGauche = false;
    }
    else
        cg=0;
    cd=cg;

    if(x < 128)
    {
        if(cg>=128-x)
            cg-=128-x;
        else
            cg=0;
        if((int16_t)cd+(int16_t)(128-x)<=255)
            cd+=128-x;
        else
            cd=255;
    }
    else if(x > 128)
    {
        if(cd>=x-128)
            cd-=x-128;
        else
            cd=0;

        if((int16_t)cg+(int16_t)(x-128)<=255)
            cg+=(x-128);
        else
            cg=255;
    }

    return mCommand;
}
