#include "PCB_Moteur.hpp"

using namespace std;

PCB_Moteur::PCB_Moteur() : mSpeedFactor(1), mEtatUS(0), mUpdateUS(false)
{
    mCommand.mAgent = Agent::I2C;
    mCommand.i2c_p.type = PCommand::I2C_Parameters::I2C_Command::MicroC_SetCommandMotor;
    mCommand.i2c_p.motorP.vitesseDroite = 0;
    mCommand.i2c_p.motorP.vitesseGauche = 0;
    mCommand.i2c_p.motorP.renvoieDistance = true;
}

PCommand PCB_Moteur::updateWithJoystick(struct PEvent::Network_Parameters::MotionParameters::JoystickParameters joystickData)
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
        if (motorP.directionDroite != true || motorP.directionGauche != true)
        {
            motorP.directionDroite = true;
            motorP.directionGauche = true;
            mUpdateUS = true;
            mEtatUS = 1;

        }
    }
    else if(y < 128)
    {
        if(y!=0)
            cg=((128-y))*2;
        else
            cg=255;
        if (motorP.directionDroite != false || motorP.directionGauche != false)
        {
            motorP.directionDroite = false;
            motorP.directionGauche = false;
            mUpdateUS = true;
            mEtatUS = 2;

        }

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
    if (mSpeedFactor !=0 || mSpeedFactor!= 1)
    {
        cg *= (double)((255/((1-mSpeedFactor)*cg+255*mSpeedFactor))*mSpeedFactor);
        cd *= (double)((255/((1-mSpeedFactor)*cd+255*mSpeedFactor))*mSpeedFactor);
    }
    return mCommand;
}

PCommand PCB_Moteur::updateWithUS(PEvent::US_Parameters::US_Seuil seuil)
{
    bool vitesseProgressive = true;
    switch(seuil)
    {
        case PEvent::US_Parameters::US_Seuil::NoObstacle :
        {
            mSpeedFactor = 1;
            cout << "no" <<endl;
            break;
        }
        case PEvent::US_Parameters::US_Seuil::Seuil1m :
        {
            mSpeedFactor = 0.8;
            cout << "1m" <<endl;
            break;
        }
        case PEvent::US_Parameters::US_Seuil::Seuil50cm :
        {
            mSpeedFactor = 0.5;
            cout << "50cm" <<endl;
            break;
        }
        case PEvent::US_Parameters::US_Seuil::Seuil25cm :
        {
            mSpeedFactor = 0.2;
            cout << "25cm" <<endl;
            break;
        }
        case PEvent::US_Parameters::US_Seuil::Seuil10cm :
        {
            mSpeedFactor = 0;
            cout << "10cm" <<endl;
            vitesseProgressive = false;

            break;
        }
        case PEvent::US_Parameters::US_Seuil::SeuilUtilisateur:
        {
            break;
        }
    }
    mCommand.i2c_p.motorP.vitesseProgressiveDroite =  vitesseProgressive;
    mCommand.i2c_p.motorP.vitesseProgressiveGauche = vitesseProgressive;
    if (mSpeedFactor !=0 || mSpeedFactor!= 1)
    {
        mCommand.i2c_p.motorP.vitesseGauche *= (double)((255/((1-mSpeedFactor)*mCommand.i2c_p.motorP.vitesseGauche+255*mSpeedFactor))*mSpeedFactor);
        mCommand.i2c_p.motorP.vitesseDroite *= (double)((255/((1-mSpeedFactor)*mCommand.i2c_p.motorP.vitesseDroite+255*mSpeedFactor))*mSpeedFactor);
    }
    return mCommand;
}
PCommand PCB_Moteur::updateWithRotation(PEvent::Network_Parameters::MotionParameters::RotationParameters param)
{

    PCommand::I2C_Parameters::MotorParameters &motorP = mCommand.i2c_p.motorP;

    motorP.vitesseGauche = 128;
    motorP.vitesseDroite = 128;

    motorP.vitesseProgressiveDroite =  true;
    motorP.vitesseProgressiveGauche = true;

    if(param == PEvent::Network_Parameters::MotionParameters::RotationParameters::Trigo)
    {
        motorP.directionGauche = false;
        motorP.directionDroite = true;
    }
    else if(param == PEvent::Network_Parameters::MotionParameters::RotationParameters::Anti)
    {
        motorP.directionGauche = true;
        motorP.directionDroite = false;
    }
    else if(param == PEvent::Network_Parameters::MotionParameters::RotationParameters::Stop)
    {
        motorP.vitesseGauche = 0;
        motorP.vitesseDroite = 0;
    }
    mEtatUS = 0;
    return mCommand;
}

uint8_t PCB_Moteur::getEtatUS()
{
    if (mUpdateUS)
    {
        mUpdateUS = false;
        return mEtatUS;
    }
    else
        return 3;
}

void PCB_Moteur::setEtatUS(uint8_t etatUS)
{
    if (etatUS== 0 || etatUS == 1 || etatUS == 2)
        mEtatUS = etatUS;
}
bool PCB_Moteur::getLeftDirection() const
{
    return mCommand.i2c_p.motorP.directionGauche;
}

bool PCB_Moteur::getRightDirection() const
{
    return mCommand.i2c_p.motorP.directionDroite;
}

