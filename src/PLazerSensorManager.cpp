#include "PLazerSensorManager.hpp"
#include "PI2C.hpp"

PLazerSensorManager::PLazerSensorManager(PI2C *ParentI2C) : mParentI2C(ParentI2C),
                                                            mFrontLaserSensor(ParentI2C), mRightLaserSensor(ParentI2C), mLeftLaserSensor(ParentI2C),
                                                            mPm(PGpioManager::getInstance())
{
    //ctor
}

PLazerSensorManager::~PLazerSensorManager()
{
    //dtor
}

void PLazerSensorManager::PLazerSensorManager_Init()
{
    mPm.write(Pin::XSHUT_D,0);
    mPm.write(Pin::XSHUT_G,0);

    mPm.write(Pin::XSHUT_S,1);
    mParentI2C->mI2C_Device = i2c_Device::LaserAvant;
    if(mFrontLaserSensor.SetAddress(PLazerSensor::AddressLaser::Avant) == 0)
    {
        mFrontLaserSensor.Init();
        if (mInitAvantOK == 0)
            cout << "la configuration du laser avant a echouer" << endl;
    }

    mPm.write(Pin::XSHUT_G,1);
    mParentI2C->mI2C_Device = i2c_Device::LaserGauche;
    if(mLeftLaserSensor.SetAddress(PLazerSensor::AddressLaser::Gauche) == 0)
    {
        mInitGaucheOK = mLeftLaserSensor.Init();
        if (mInitGaucheOK == 0)
            cout << "la configuration du laser gauche a echouer" << endl;
    }

    mPm.write(Pin::XSHUT_D,1);
    mParentI2C->mI2C_Device = i2c_Device::LaserDroit;
    if(mFrontLaserSensor.SetAddress(PLazerSensor::AddressLaser::Droit) == 0)
    {
        mRightLaserSensor.Init();
        if (mInitDroitOK == 0)
            cout << "la configuration du laser droit a echouer" << endl;
    }

}

void PLazerSensorManager::PLazerSensorManager_Shutdown()
{
    mPm.write(Pin::XSHUT_S,0);
    mPm.write(Pin::XSHUT_D,0);
    mPm.write(Pin::XSHUT_G,0);
}

void PLazerSensorManager::PLazerSensorManager_Measure(PLazerSensor::AddressLaser addressLazer)
{

}
