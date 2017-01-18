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
            WriteFailed
        };

        I2C_Event type;
    };

    struct Network_Parameters
    {
        enum class Network_Event
        {
            ClientConnected,
            ClientDisconnected,
            JoystickMoved
        };

        struct JoystickParameters
        {
            uint8_t x;
            uint8_t y;
        };

        Network_Event type;

        union
        {
            JoystickParameters joystick;
        };
    };

    Agent mAgent;

    union
    {
        I2C_Parameters i2c_p;
        Network_Parameters network_p;
    };
};

#endif // PEVENT_HPP
