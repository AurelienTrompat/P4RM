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

    mPm.write(Pin::XSHUT_S,0);
    this_thread::sleep_for(chrono::milliseconds(1));
   /* mParentI2C->mI2C_Device = i2c_Device::LaserAvant;
    if(mFrontLaserSensor.SetAddress(PLazerSensor::AddressLaser::Avant) == 0)
    {
        mInitAvantOK = mFrontLaserSensor.Init();
        if (mInitAvantOK == 0)
            cout << "la configuration du laser avant a echouer" << endl;
        mFrontLaserSensor.setTimeout(500);
        // lower the return signal rate limit (default is 0.25 MCPS)
        mFrontLaserSensor.setSignalRateLimit(0.1);
        // increase laser pulse periods (defaults are 14 and 10 PCLKs)
        mFrontLaserSensor.setVcselPulsePeriod(PLazerSensor::VcselPeriodPreRange, 18);
        mFrontLaserSensor.setVcselPulsePeriod(PLazerSensor::VcselPeriodFinalRange, 14);
        mFrontLaserSensor.setMeasurementTimingBudget(20000);
    }

    mPm.write(Pin::XSHUT_G,1);
    mParentI2C->mI2C_Device = i2c_Device::LaserGauche;
    if(mLeftLaserSensor.SetAddress(PLazerSensor::AddressLaser::Gauche) == 0)
    {
        mInitGaucheOK = mLeftLaserSensor.Init();
        if (mInitGaucheOK == 0)
            cout << "la configuration du laser gauche a echouer" << endl;
        mLeftLaserSensor.setTimeout(500);
        // lower the return signal rate limit (default is 0.25 MCPS)
        mLeftLaserSensor.setSignalRateLimit(0.1);
        // increase laser pulse periods (defaults are 14 and 10 PCLKs)
        mLeftLaserSensor.setVcselPulsePeriod(PLazerSensor::VcselPeriodPreRange, 18);
        mLeftLaserSensor.setVcselPulsePeriod(PLazerSensor::VcselPeriodFinalRange, 14);
        mLeftLaserSensor.setMeasurementTimingBudget(20000);
    }
*/
    mPm.write(Pin::XSHUT_S,1);

    this_thread::sleep_for(chrono::milliseconds(1));
    mParentI2C->mI2C_Device = i2c_Device::LaserDroit;
    if(mFrontLaserSensor.SetAddress(PLazerSensor::AddressLaser::Droit) == 0)
    {
        this_thread::sleep_for(chrono::milliseconds(100));
        mParentI2C->SetAdresse(0x31);
        cout << "changement adresse OK !" << endl;
        mInitDroitOK = mRightLaserSensor.Init();
        if (mInitDroitOK == 0)
            cout << "la configuration du laser droit a echouer" << endl;
        cout << "init OK" <<endl;
        mRightLaserSensor.setTimeout(500);
        // lower the return signal rate limit (default is 0.25 MCPS)
        mRightLaserSensor.setSignalRateLimit(0.1);
        // increase laser pulse periods (defaults are 14 and 10 PCLKs)
        mRightLaserSensor.setVcselPulsePeriod(PLazerSensor::VcselPeriodPreRange, 18);
        mRightLaserSensor.setVcselPulsePeriod(PLazerSensor::VcselPeriodFinalRange, 14);
        mRightLaserSensor.setMeasurementTimingBudget(20000);
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
    if (((std::chrono::duration<double>)(Chrono::now()- mChronoLaser)).count() > 1)
    {
        mParentI2C->SetAdresse(0x31);
        mChronoLaser = Chrono::now();
        switch (addressLazer)
        {
            case PLazerSensor::AddressLaser::Droit:
            {
                mRightLaserSensor.Measure();
                break;
            }
            default :
            {
                break;
            }
        }
    }
}
