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
};

#endif // PI2C_HPP
