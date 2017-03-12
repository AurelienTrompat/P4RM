#ifndef PROBOT_HPP
#define PROBOT_HPP

#include <iostream>
#include <atomic>

#include "PMaster.hpp"
#include "PGpioManager.hpp"
#include "PNetwork.hpp"
#include "PI2C.hpp"
#include "PPositionTracker.hpp"
#include "PUltrasonicSensor.hpp"

#include "PCB_Moteur.hpp"

class PRobot : public PMaster
{
    public:
        PRobot();
        ~PRobot();

    private:
        void preRun();
        void run();
        void postRun();

        void handleEvent(const PEvent &event);

        void handleNetworkEvent(const PEvent &event);
        void handleI2CEvent(const PEvent &event);
        void handleUSEvent(const PEvent &event);
        void handlePosTrackerEvent(const PEvent &event);

        void convertAndRelayDist(uint16_t leftDist, uint16_t rightDist);

        int16_t fromStepToCentimeter(int16_t);

    private:
        PGpioManager &mPm;
        PNetwork mNetwork;
        PI2C mI2C;
        PUltrasonicSensor mUS;
        PPositionTracker mPosTracker;

        PCB_Moteur mCB_Moteur;

        unsigned long mDD;
        unsigned long mDG;
};

#endif // PROBOT_HPP
