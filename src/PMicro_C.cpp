#include "PMicro_C.hpp"
#include "PI2C.hpp"
PMicro_C::PMicro_C(PI2C *ParentI2C) : mParentI2C(ParentI2C), mMoteurGErreur(false), mMoteurDErreur(false), mArretGauche(0), mArretDroit(0), mActiverRenvoieDist(false), mRenvoieDistance(false)
{
    //ctor

}

PMicro_C::~PMicro_C()
{
    //dtor
}

void PMicro_C::MicroC_WriteCmd(const struct PCommand::I2C_Parameters &i2c_p)
{
    // Moteur Gauche
    mCmdMoteur.CmdMGauche   =  (1 << 5)
                               + (0 << 4)
                               + (i2c_p.motorP.renvoieDistance << 3)
                               + (1 << 2)
                               + (i2c_p.motorP.directionGauche << 1)
                               + (i2c_p.motorP.vitesseProgressiveGauche);

    mCmdMoteur.VitesseGauche = i2c_p.motorP.vitesseGauche;

    //Moteur Droit
    mCmdMoteur.CmdMDroit    = (1 << 5)
                              + (0 << 4)
                              + (i2c_p.motorP.renvoieDistance << 3)
                              + (1 << 2)
                              + (i2c_p.motorP.directionDroite << 1)
                              + (i2c_p.motorP.vitesseProgressiveDroite);

    mCmdMoteur.VitesseDroite = i2c_p.motorP.vitesseDroite;
    mParentI2C->mI2C_Command = i2c_Command::SetCommandMotor;

    if (i2c_p.motorP.renvoieDistance && !mRenvoieDistance && !mMoteurGErreur && !mMoteurDErreur)
    {
        mActiverRenvoieDist = true;
    }

}

void PMicro_C::MicroC_ShutdownMoteur()
{
    union i2c_smbus_data data;
    data.byte = 0;

// Moteur Gauche
    mParentI2C->mI2C_Device = i2c_Device::MoteurGauche;
    mParentI2C->SetAdresse(0x10);
    mParentI2C->BusAccess(I2C_SMBUS_WRITE,(1 << 5),I2C_SMBUS_BYTE_DATA, &data);

//Moteur Droit
    mParentI2C->mI2C_Device = i2c_Device::MoteurDroit;
    mParentI2C->SetAdresse(0x20);
    mParentI2C->BusAccess(I2C_SMBUS_WRITE,(1 << 5),I2C_SMBUS_BYTE_DATA, &data);
    mRenvoieDistance = false;
}

void PMicro_C::MicroC_ReadErreurEtVitesse()
{
    union i2c_smbus_data data;

    mParentI2C->mI2C_Device = i2c_Device::MoteurGauche;
    mParentI2C->SetAdresse(0x10);
    int lectureGauche = mParentI2C->BusAccess(I2C_SMBUS_READ, 0x00, I2C_SMBUS_WORD_DATA, &data);
    uint16_t distanceGauche = ((data.word & 0x7f00) >> 8) + ((data.word & 0x0040) << 1);
    int erreurMoteurGauche = MicroC_CheckError((uint8_t)(data.word & 0x0007));
    if (erreurMoteurGauche == -1)
        mMoteurGErreur = true;
    else
        mMoteurGErreur = false;

    mParentI2C->mI2C_Device = i2c_Device::MoteurDroit;
    mParentI2C->SetAdresse(0x20);
    int lectureDroite = mParentI2C->BusAccess(I2C_SMBUS_READ, 0x00, I2C_SMBUS_WORD_DATA, &data);
    uint16_t distanceDroit = ((data.word & 0x7f00) >> 8) + ((data.word & 0x0040) << 1);
    int erreurMoteurDroit = MicroC_CheckError((uint8_t)(data.word & 0x0007));
    if(erreurMoteurDroit == -1)
        mMoteurDErreur = true;
    else
        mMoteurDErreur = false;

    if (lectureGauche != -1 && lectureDroite != -1 && erreurMoteurGauche == 0 && erreurMoteurDroit == 0)
    {
        mParentI2C->SendEvent(i2c_Event::I2C_DistanceParcouru, distanceGauche, distanceDroit);
    }
}

int PMicro_C::MicroC_CheckError(uint8_t error)
{
    if (error != 0)
    {
        if(!mMoteurGErreur && !mMoteurDErreur)
        {
            if((error & 0x02) == 0x02)
                mParentI2C->SendEvent(i2c_Event::I2C_ErrorBattery);
            if((error & 0x04) == 0x04)
                mParentI2C->SendEvent(i2c_Event::I2C_ErrorMotorDriver);
            if((error & 0x01) == 0x01)
                mParentI2C->SendEvent(i2c_Event::I2C_ErrorTimeOut);
            MicroC_ShutdownMoteur();
        }
        return -1;
    }
    else
    {
        return 0;
    }
}

void PMicro_C::MicroC_DistanceArret()
{
    union i2c_smbus_data data;

//Moteur Gauche
    mParentI2C->mI2C_Device = i2c_Device::MoteurGauche;
    mParentI2C->SetAdresse(0x10);
    int lectureGauche = mParentI2C->BusAccess(I2C_SMBUS_READ, 0x00, I2C_SMBUS_WORD_DATA, &data);
    uint16_t distanceArretGauche = 255*((data.word & 0x0038) >> 3) + ((data.word & 0x7f00) >> 8) + ((data.word & 0x0040) << 1);

//Moteur Droit
    mParentI2C->mI2C_Device = i2c_Device::MoteurDroit;
    mParentI2C->SetAdresse(0x20);
    int lectureDroit = mParentI2C->BusAccess(I2C_SMBUS_READ, 0x00, I2C_SMBUS_WORD_DATA, &data);
    uint16_t distanceArretDroit = 255*((data.word & 0x0038) >> 3) + ((data.word & 0x7f00) >> 8) + ((data.word & 0x0040) << 1);

    if (lectureGauche != -1 && lectureDroit != -1 && distanceArretGauche == mArretGauche && distanceArretDroit == mArretDroit)
    {
        mParentI2C->SendEvent(i2c_Event::I2C_DistanceArret,distanceArretGauche,distanceArretDroit);
        mRenvoieDistance=false;
    }
    mArretGauche = distanceArretGauche;
    mArretDroit = distanceArretDroit;
}



void PMicro_C::MicroC_RAZDefault()
{
    union i2c_smbus_data data;
    data.byte = 0;

// Moteur Gauche
    mParentI2C->mI2C_Device = i2c_Device::MoteurGauche;
    mParentI2C->SetAdresse(0x10);
    mParentI2C->BusAccess(I2C_SMBUS_WRITE,((1 << 5)+(1 << 4)),I2C_SMBUS_BYTE_DATA, &data);

//Moteur Droit
    mParentI2C->mI2C_Device = i2c_Device::MoteurDroit;
    mParentI2C->SetAdresse(0x20);
    mParentI2C->BusAccess(I2C_SMBUS_WRITE,((1 << 5)+(1 << 4)),I2C_SMBUS_BYTE_DATA, &data);

    mMoteurGErreur = false;
    mMoteurDErreur = false;
}
void PMicro_C::MicroC_VerifDefault()
{
    union i2c_smbus_data data;

    //Moteur Gauche
    mParentI2C->SetAdresse(0x10);
    mParentI2C->mI2C_Device = i2c_Device::MoteurGauche;
    if(mParentI2C->BusAccess(I2C_SMBUS_READ,0,I2C_SMBUS_BYTE, &data) != -1)
    {
        if(MicroC_CheckError((uint8_t)(data.byte & 0x0007))==-1)
            mMoteurGErreur = true;
        else
            mMoteurGErreur = false;
    }
    //Moteur Droit
    mParentI2C->mI2C_Device = i2c_Device::MoteurDroit;
    mParentI2C->SetAdresse(0x20);
    if(mParentI2C->BusAccess(I2C_SMBUS_READ,0,I2C_SMBUS_BYTE, &data) != -1)
    {
        if(MicroC_CheckError((uint8_t)(data.byte & 0x07))==-1)
            mMoteurDErreur = true;
        else
            mMoteurDErreur = false;
    }
}

void PMicro_C::MicroC_ActiverRenvoieDistance()
{
    if (mActiverRenvoieDist)
    {
        mRenvoieDistance = true;
        mChronoDistance = Chrono::now();
        mActiverRenvoieDist = false;
    }

}

void PMicro_C::MicroC_DemmanderDistance()
{
    std::chrono::duration<double> diff = Chrono::now() - mChronoDistance;
    if ((diff.count() > 0.05))
    {
        mChronoDistance = Chrono::now();
        if (mRenvoieDistance)
        {
            if (!mMoteurGErreur && !mMoteurDErreur)
                MicroC_ReadErreurEtVitesse();
            else
                MicroC_DistanceArret();
        }
        else
        {
            MicroC_VerifDefault();
        }
    }
}

void PMicro_C::MicroC_Ping()
{
    union i2c_smbus_data data;
    std::chrono::duration<double> diff = Chrono::now() - mChronoPing;
    if (diff.count() > 0.1)
    {
        mChronoPing = Chrono::now();
        if (((mCmdMoteur.CmdMGauche & (1 << 2)) == (1 << 2)) && ((mCmdMoteur.CmdMGauche & (1 << 2)) == (1 << 2)) && (!mMoteurGErreur && !mMoteurDErreur))
        {
            //Moteur Gauche
            data.byte = mCmdMoteur.VitesseGauche;
            mParentI2C->SetAdresse(0x10);
            mParentI2C->mI2C_Device = i2c_Device::MoteurGauche;
            mParentI2C->BusAccess(I2C_SMBUS_WRITE,mCmdMoteur.CmdMGauche,I2C_SMBUS_BYTE_DATA, &data);

            //Moteur Droit
            data.byte = mCmdMoteur.VitesseDroite;
            mParentI2C->mI2C_Device = i2c_Device::MoteurDroit;
            mParentI2C->SetAdresse(0x20);
            mParentI2C->BusAccess(I2C_SMBUS_WRITE,mCmdMoteur.CmdMDroit,I2C_SMBUS_BYTE_DATA, &data);
        }
        else
        {
            MicroC_ShutdownMoteur();
        }
    }
}

void PMicro_C::MicroC_SetCommandMoteur()
{
    union i2c_smbus_data data;
    if (!mMoteurGErreur && !mMoteurDErreur)
    {
        //Moteur Gauche
        data.byte = mCmdMoteur.VitesseGauche;
        mParentI2C->SetAdresse(0x10);
        mParentI2C->mI2C_Device = i2c_Device::MoteurGauche;
        mParentI2C->BusAccess(I2C_SMBUS_WRITE,mCmdMoteur.CmdMGauche,I2C_SMBUS_BYTE_DATA, &data);

        //Moteur Droit
        data.byte = mCmdMoteur.VitesseDroite;
        mParentI2C->mI2C_Device = i2c_Device::MoteurDroit;
        mParentI2C->SetAdresse(0x20);
        mParentI2C->BusAccess(I2C_SMBUS_WRITE,mCmdMoteur.CmdMDroit,I2C_SMBUS_BYTE_DATA, &data);
    }
}
