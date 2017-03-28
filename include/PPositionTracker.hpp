#ifndef PPOSITIONTRACKER_HPP
#define PPOSITIONTRACKER_HPP
#include <math.h>
#include "PAgent.hpp"


class PPositionTracker : public PAgent
{
    public:
        PPositionTracker();
        ~PPositionTracker();

    private:
        void handleCommand(const PCommand &command);
        void preRun();
        void run();
        void postRun();

        void updatePos(const PCommand &command);

    private:
        PEvent mEvent;

    const float mDistInterWheel = 325000/(123*M_PI);
    float mRadius;
    float mAngle;
    float mDxp;
    float mDyp;
    double mDx;
    double mDy;
};

#endif // PPOSITIONTRACKER_HPP
