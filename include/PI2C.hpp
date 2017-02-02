#ifndef PI2C_HPP
#define PI2C_HPP

#include <iostream>
#include <atomic>
#include <bitset>

#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>

#include "PCommand.hpp"
#include "PAgent.hpp"

typedef PCommand::I2C_Parameters::I2C_Command i2c_Command;
typedef PEvent::I2C_Parameters::I2C_Event i2c_Event;
typedef PEvent::I2C_Parameters::I2C_Device i2c_Device;
typedef std::chrono::high_resolution_clock Chrono;


class PI2C : public PAgent
{
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
        int BusAccess (bool rw, uint8_t command, int dataSize, union i2c_smbus_data *data);

        //Microcontroleurs
        void MicroC_WriteCmd(const struct PCommand::I2C_Parameters &i2c_p);
        void MicroC_ShutdownMoteur();
        void MicroC_ReadErreurEtVitesse();
        int MicroC_CheckError(uint8_t error);
        void MicroC_DistanceArret();
        void MicroC_RAZDefault();
        void MicroC_VerifDefault();

    private:
        int mFd;
        std::atomic_bool mMoteurGErreur;
        std::atomic_bool mMoteurDErreur;
        uint16_t mDistanceArret;
        uint16_t mArretGauche;
        uint16_t mArretDroit;
        std::atomic_bool mNewCommand;
        i2c_Command mI2C_Command;
        i2c_Device mI2C_Device;

        struct CommandeMoteur
        {
            uint8_t CmdMGauche;
            uint8_t CmdMDroit;
            uint8_t VitesseGauche;
            uint8_t VitesseDroite;
        };

        struct CommandeMoteur mCmdMoteur;

        std::atomic_bool mActiverRenvoieDist;
        std::atomic_bool mRenvoieDistance;
        Chrono::time_point mChronoDistance;
        Chrono::time_point mChronoPing;
};

#endif // PI2C_HPP
