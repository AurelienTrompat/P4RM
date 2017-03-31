#ifndef PI2C_HPP
#define PI2C_HPP

#include <iostream>
#include <atomic>

#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>

#include "PCommand.hpp"
#include "PAgent.hpp"
#include "PMicro_C.hpp"
#include "PModule9DOF.hpp"
#include "PLazerSensorManager.hpp"
#include "PLazerSensor.hpp"

typedef PCommand::I2C_Parameters::I2C_Command i2c_Command;
typedef PEvent::I2C_Parameters::I2C_Event i2c_Event;
typedef PEvent::I2C_Parameters::I2C_Device i2c_Device;

class PMicro_C;
class PModule9DOF;
class PLazerSensorManager;
class PLazerSensor;

class PI2C : public PAgent
{
    friend PMicro_C;
    friend PModule9DOF;
    friend PLazerSensor;
    friend PLazerSensorManager;

    public:
        PI2C();
        ~PI2C();
        std::string fromDeviceToString(const i2c_Device device);

    private:
        void preRun();
        void run();
        void postRun();
        void handleCommand(const PCommand &command);
        void OpenI2C();
        void SetAdresse(uint8_t adresse);
        void SendEvent(PEvent::I2C_Parameters::I2C_Event typeEvent);
        void SendEvent(PEvent::I2C_Parameters::I2C_Event typeEvent, uint16_t distanceGauche, uint16_t distanceDroite);
        void SendEvent(PEvent::I2C_Parameters::I2C_Event typeEvent, double data);
        int BusAccess (bool rw, uint8_t command, int dataSize, union i2c_smbus_data *data);

    private:
        PMicro_C mMicroC;
        PModule9DOF mModule9DOF;
        PLazerSensorManager mLazerSensorManager;
        int mFd;
        i2c_Device mI2C_Device;
        i2c_Command mI2C_Command;
        std::atomic_bool mNewCommand;





};

#endif // PI2C_HPP
