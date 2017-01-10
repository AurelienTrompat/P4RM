#ifndef PEVENT_HPP
#define PEVENT_HPP


class PEvent
{
    public:

        struct JoystickEvent
        {
            signed char x;
            signed char y;
        };

        enum class Type
        {
            Quit,
            ClientConnected,
            ClientDisconnected,
            Joystick,
            I2C_NotOpen,
            I2C_Open,
            I2C_SetAdresseFailed,
            I2C_SetAdresseSuccess,
            I2C_WriteFailed,
            I2C_WriteSucess
        };

        Type mType;

        union
        {
            JoystickEvent joystick;
        };
};

#endif // PEVENT_HPP
