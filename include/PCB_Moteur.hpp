#ifndef PCB_MOTEUR_HPP
#define PCB_MOTEUR_HPP

#include <PCommandBuilder.hpp>


class PCB_Moteur : public PCommandBuilder
{
    public:
        PCB_Moteur();

        PCommand updateWithJoystick(struct PEvent::Network_Parameters::MotionParameters::JoystickParameters joystickData);
        PCommand updateWithRotation(PEvent::Network_Parameters::MotionParameters::RotationParameters param);


};

#endif // PCB_MOTEUR_HPP
