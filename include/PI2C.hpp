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

typedef PCommand::I2C_Parameters::I2C_Command i2c_Command;

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
        void MicroC_WriteCmd(const struct PCommand::I2C_Parameters &i2c_p);
        void MicroC_ShutdownMoteur();

    private:
        int mFd;
        std::atomic_bool mNewCommand;
        i2c_Command mI2C_Command;

        struct CommandeMoteur
        {
            uint8_t CmdMGauche;
            uint8_t CmdMDroit;
            uint8_t VitesseGauche;
            uint8_t VitesseDroite;
        };
        struct CommandeMoteur mCmdMoteur;

};

#endif // PI2C_HPP
