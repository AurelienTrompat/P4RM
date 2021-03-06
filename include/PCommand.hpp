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
            MicroC_SetCommandMotor,
            MicroC_StopMoteur,
            MicroC_VerifDefaultMotor,
            MicroC_RAZDefaultMotor,
            Gyro_Start,
            Gyro_Stop,
            Axel_Start,
            Axel_Stop,
            Magn_Start,
            Magn_Stop,
            LaserAskDataFromFront,
            LaserAskDataFromLeft,
            LaserAskDataFromRight
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

    struct US_Parameters
    {
        enum class US_Command
        {
            StartAvant,
            StartArriere,
            StopUS,
            SetSeuil,
            GetDistance,
            Reset
        };
        US_Command type;
        uint16_t seuil;
    };

    struct PositionTracker_Parameters
    {
        enum class PositionTracker_Command
        {
            UpdatePosition,
            UpdateAngle, //Gyro
            ResetPosition
        };
        PositionTracker_Command type;

        struct TraveledDist
        {
            int16_t leftDist;
            int16_t rightDist;
        };
        union
        {
            TraveledDist traveledDist;
            double angleGyro;
        };

    };

    struct Network_Parameters
    {
        enum class Network_Command
        {
            NewPosition,
            NewSensorData,
            UsEnabled,
            UsDisabled,
            ServoEnabled,
            ServoDisabled
        };

        struct Position
        {
            int16_t x;
            int16_t y;
            int16_t phi;
        };

        struct NewSensorData_Parameters
        {
            enum class Sensor
            {
                LaserGauche,
                LaserAvant,
                LaserDroit,
                Accel,
                Magn,
            };

            Sensor sensor;

            union
            {
                uint16_t valueInt;
                double valueDouble;
            };
        };

        Network_Command type;

        union
        {
            struct Position pos;
            struct NewSensorData_Parameters newSensorData_p;
        };

    };

    Agent mAgent;

    union
    {
        struct I2C_Parameters i2c_p;
        struct US_Parameters us_p;
        struct PositionTracker_Parameters posTracker_p;
        struct Network_Parameters network_p;
    };
};

#endif // PCOMMAND_HPP
