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
            Joystick
        };

        Type mType;

        union
        {
            JoystickEvent joystick;
        };
};

#endif // PEVENT_HPP
