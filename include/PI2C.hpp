#ifndef PI2C_HPP
#define PI2C_HPP

#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>

#include "PCommand.hpp"
#include "PAgent.hpp"



class PI2C : public PAgent
{
    public:
        PI2C();
        ~PI2C();

    private:
        void preRun();
        void run();
        void postRun();
        void handleCommand(const PCommand &command);
        void OpenI2C();
        void SetAdresse(uint8_t adresse);
        void SendEvent(PEvent::Type typeEvent);
        int BusAccess (bool rw, uint8_t command, int dataSize, union i2c_smbus_data *data);
        void WriteData(const struct PCommand::I2C_Parameters &i2c_p);
    private:
        int mFd;
};

#endif // PI2C_HPP
