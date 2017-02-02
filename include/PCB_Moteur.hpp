#ifndef PCB_MOTEUR_HPP
#define PCB_MOTEUR_HPP

#include <iostream>
#include <PCommandBuilder.hpp>


class PCB_Moteur : public PCommandBuilder
{
    public:
        PCB_Moteur();

        PCommand updateWithJoystick(struct PEvent::Network_Parameters::JoystickParameters joystickData);
        PCommand updateWithUS(PEvent::US_Parameters::US_Seuil seuil);

    private:
        float mSpeedFactor;

};

#endif // PCB_MOTEUR_HPP
