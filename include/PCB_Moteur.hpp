#ifndef PCB_MOTEUR_HPP
#define PCB_MOTEUR_HPP

#include <PCommandBuilder.hpp>


class PCB_Moteur : public PCommandBuilder
{
    public:
        PCB_Moteur();

        PCommand updateWithJoystick(struct PEvent::Network_Parameters::JoystickParameters joystickData);


};

#endif // PCB_MOTEUR_HPP
