#ifndef PCOMMAND_HPP
#define PCOMMAND_HPP

#include "PDefines.hpp"

class PCommand
{
public:
    struct I2C_Parameters
    {
        enum class I2C_Command
        {
            SetCommandMotor,
            StopMoteur,
            VerifDefaultMotor,
            RAZDefaultMotor,
            Giro,
            Axel,
            Magn,
            Laser,
        };

        struct MotorParameters
        {
            uint8_t vitesseGauche;
            bool directionGauche;
            bool vitesseProgressiveGauche;

            uint8_t vitesseDroite;
            bool directionDroite;
            bool vitesseProgressiveDroite;

            bool renvoieDistance;
        };

        I2C_Command type;
        union
        {
            struct MotorParameters motorP;
        };

    };

    Agent mAgent;

    union
    {
        struct I2C_Parameters i2c_p;
    };
};

#endif // PCOMMAND_HPP
