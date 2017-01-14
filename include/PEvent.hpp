#ifndef PEVENT_HPP
#define PEVENT_HPP


class PEvent
{
    public:

        struct JoystickEvent
        {
            uint8_t x;
            uint8_t y;
        };

        enum class Type
        {
            Quit,
            ClientConnected,
            ClientDisconnected,
            Joystick,
            I2C_NotOpen,
            I2C_SetAdresseFailed,
            I2C_WriteFailed
        };

        Type mType;

        union
        {
            JoystickEvent joystick;
        };
};

#endif // PEVENT_HPP
