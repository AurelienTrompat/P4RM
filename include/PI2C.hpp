#ifndef PI2C_HPP
#define PI2C_HPP

#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

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

    private:
        int fd;
};

#endif // PI2C_HPP
