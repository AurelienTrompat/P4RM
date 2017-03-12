#ifndef PCB_MOTEUR_HPP
#define PCB_MOTEUR_HPP

#include <iostream>
#include <PCommandBuilder.hpp>


class PCB_Moteur : public PCommandBuilder
{
    public:
        PCB_Moteur();

        PCommand updateWithJoystick(struct PEvent::Network_Parameters::MotionParameters::JoystickParameters joystickData);
        PCommand updateWithRotation(PEvent::Network_Parameters::MotionParameters::RotationParameters param);
        PCommand updateWithUS(PEvent::US_Parameters::US_Seuil seuil);

        uint8_t getEtatUS();
        void setEtatUS(uint8_t etatUS);

        bool getLeftDirection() const;
        bool getRightDirection() const;

    private:
        double mSpeedFactor;
        uint8_t mEtatUS;
        bool mUpdateUS;

};

#endif // PCB_MOTEUR_HPP
