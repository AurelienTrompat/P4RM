#ifndef PMODULE9DOF_HPP
#define PMODULE9DOF_HPP

#include <linux/i2c.h>

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

        void Gyro_Init();
        void Gyro_Start();
        void Gyro_Stop();
        void Gyro_Shutdown();
        void Gyro_CheckAngle();
        void Gyro_RAZDefault();

    private:
        PI2C *mParentI2C;

        Chrono::time_point mChronoCheckGyroAngle;
        double positionAngulaire;
};

#endif // PMODULE9DOF_HPP

