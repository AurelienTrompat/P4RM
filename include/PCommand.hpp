#ifndef PCOMMAND_HPP
#define PCOMMAND_HPP

class PCommand
{
public:
    enum class Agent
    {
        Network,
        I2C
    };

    struct I2C_Parameters
    {
        enum class I2C_Command
        {
            SetCommandMotor,
            StopMoteur,
            Giroscope,
            AxelEtMagn,
            Laser
        };

        struct MotorParameters
        {
            uint8_t vitesseGauche;
            bool directionGauche;
            bool renvoieDistanceGauche;
            bool vitesseProgressiveGauche;
            bool RAZdefaultGauche;

            uint8_t vitesseDroite;
            bool directionDroite;
            bool renvoieDistanceDroite;
            bool vitesseProgressiveDroite;
            bool RAZdefaultDroite;
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
