#ifndef PEVENT_HPP
#define PEVENT_HPP


class PEvent
{
    protected:

        struct JoystickEvent
        {
            signed char x;
            signed char y;
        };

        enum class EventType
        {
            ClientConnected,
            ClientDisconnected,
            Joystick
        };

    public:
        EventType mEventType;

        union
        {
            JoystickEvent joystick;
        };
};

#endif // PEVENT_HPP
