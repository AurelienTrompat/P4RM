#ifndef PMICRO_C_HPP
#define PMICRO_C_HPP

#include <atomic>
#include <linux/i2c.h>

#include "PCommand.hpp"
#include "PAgent.hpp"

typedef PCommand::I2C_Parameters::I2C_Command i2c_Command;
typedef PEvent::I2C_Parameters::I2C_Event i2c_Event;
typedef PEvent::I2C_Parameters::I2C_Device i2c_Device;
typedef std::chrono::high_resolution_clock Chrono;

class PI2C;
class PMicro_C
{
    public:
        PMicro_C(PI2C *ParentI2C);
        ~PMicro_C();

        void MicroC_ActiverRenvoieDistance();
        void MicroC_DemmanderDistance();
        void MicroC_Ping();
        void MicroC_SetCommandMoteur();
        bool MicroC_WriteCmd(const struct PCommand::I2C_Parameters &i2c_p);
        void MicroC_ShutdownMoteur();
        void MicroC_ReadErreurEtVitesse();
        int MicroC_CheckError(uint8_t error);
        void MicroC_DistanceArret();
        void MicroC_RAZDefault();
        void MicroC_VerifDefault();

    private:
        PI2C *mParentI2C;

        std::atomic_bool mMoteurGErreur;
        std::atomic_bool mMoteurDErreur;
        uint16_t mDistanceArret;
        uint16_t mArretGauche;
        uint16_t mArretDroit;

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

#endif // PMICRO_C_HPP
