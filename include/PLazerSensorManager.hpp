#ifndef PLAZERSENSORMANAGER_HPP
#define PLAZERSENSORMANAGER_HPP

#include "PLazerSensor.hpp"
#include "PGpioManager.hpp"

class PLazerSensorManager
{
    public:
        PLazerSensorManager(PI2C *ParentI2C);
        ~PLazerSensorManager();
        void PLazerSensorManager_Init();
        void PLazerSensorManager_Shutdown();
        void PLazerSensorManager_Measure(PLazerSensor::AddressLaser addressLazer);

    private:
        PI2C *mParentI2C;
        PLazerSensor mFrontLaserSensor;
        PLazerSensor mRightLaserSensor;
        PLazerSensor mLeftLaserSensor;
        PGpioManager &mPm;
        bool mInitAvantOK;
        bool mInitDroitOK;
        bool mInitGaucheOK;
};

#endif // PLAZERSENSORMANAGER_HPP
