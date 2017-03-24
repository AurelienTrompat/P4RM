#ifndef PMODULE9DOF_HPP
#define PMODULE9DOF_HPP

#include <linux/i2c.h>
#include <cmath>

#include "PCommand.hpp"
#include "PAgent.hpp"


typedef PEvent::I2C_Parameters::I2C_Device i2c_Device;
typedef std::chrono::high_resolution_clock Chrono;
typedef PEvent::I2C_Parameters::I2C_Event i2c_Event;

class PI2C;

class PModule9DOF
{
    public:
        PModule9DOF(PI2C *ParentI2C);
        ~PModule9DOF();

        void PModule9DOF_Init();
        void PModule9DOF_Shutdown();

        void Gyro_RAZDefault();

        void Gyro_CheckAngle();
        void Magn_CheckOrientation();
        void Axel_CheckAxeleration();

        void Gyro_Stop();
        void Magn_Stop();
        void Axel_Stop();

        void Gyro_Start();
        void Magn_Start();
        void Axel_Start();

        void SetRobotImmobile(uint8_t robotImmobile);

    private:
        void Gyro_Init();
        void Gyro_Shutdown();

        void Magn_Init();
        void Magn_Shutdown();

        void Axel_Init();
        void Axel_Shutdown();

    private:
        PI2C *mParentI2C;
        bool mGyroEnable;
        bool mAxelEnable;
        bool mMagnEnable;
        Chrono::time_point mChronoCheckGyroAngle;
        Chrono::time_point mChronoCheckMagnAngle;
        Chrono::time_point mChronoCheckAxelZ;
        double mPositionAngulaire;
        atomic_bool mRobotImmobile;
};

#endif // PMODULE9DOF_HPP

