#ifndef PEVENT_HPP
#define PEVENT_HPP


class PEvent
{
public:

    struct I2C_Parameters
    {
        enum class I2C_Event
        {
            OpenFailed,
            SetAddressFailed,
            WriteFailed,
            I2C_ErrorBattery,
            I2C_ErrorMotorDriver,
            I2C_ErrorTimeOut,
            I2C_DistanceParcouru,
            I2C_DistanceArret,
            I2C_ZAxisAngularData,
            I2C_ZAxisAccelerationData,
            I2C_NewDataFromLazerSensor
        };

        enum class I2C_Device
        {
            I2C,
            MoteurGauche,
            MoteurDroit,
            Gyro,
            Axel,
            Magn,
            LaserAvant,
            LaserGauche,
            LaserDroit
        };

        union
        {
            uint8_t distanceDroite;
            uint16_t distanceArretDroite;
        };
        union
        {
            uint8_t distanceGauche;
            uint16_t distanceArretGauche;
        };

        union
        {
            double angularData;
            double accelerationData;
            uint16_t DistanceData;

        };

        I2C_Event type;
        I2C_Device device;
    };

    struct Network_Parameters
    {
        enum class Network_Event
        {
            ClientConnected,
            ClientDisconnected,
            Motion,
            ButtonRAZDefaults,
            ButtonRAZPosition
        };

        struct MotionParameters
        {
            enum class MotionType
            {
                Joystick,
                Rotation
            };


            struct JoystickParameters
            {
                uint8_t x;
                uint8_t y;
            };
            enum class RotationParameters
            {
                Trigo,
                Anti,
                Stop
            };
            MotionType type;
            union
            {
                struct JoystickParameters joystick;
                RotationParameters rotation_p;
            };
        };

        Network_Event type;

        union
        {
            struct MotionParameters motion_p;
        };
    };

    struct US_Parameters
    {
        enum class US_Event
        {
            US_Obstacle,
            US_Error,
            US_Distance
        };

        enum class US_Device
        {
           CapteurAvant,
           CapteurArriere
        };

        enum class US_Seuil
        {
            SeuilUtilisateur,
            Seuil1m,
            Seuil50cm,
            Seuil25cm,
            Seuil10cm,
            NoObstacle
        };

        US_Event type;
        US_Device device;
        US_Seuil seuil;

        uint16_t distObstacle;
    };

    struct PositionTracker_Parameters
    {
        enum class PositionTracker_Event
        {
            PosHasChanged
        };

        PositionTracker_Event type;

        struct Position
        {
            int size;
            int resolution;
            double x;
            double y;
            float phi;
        };
        Position pos;
    };

    Agent mAgent;

    union
    {
        struct I2C_Parameters i2c_p;
        struct Network_Parameters network_p;
        struct US_Parameters us_p;
        struct PositionTracker_Parameters posTracker_p;
    };
};

#endif // PEVENT_HPP
