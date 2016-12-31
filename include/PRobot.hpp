#ifndef PROBOT_HPP
#define PROBOT_HPP

#include <iostream>
#include "PMaster.hpp"
#include "PNetwork.hpp"
#include "PI2C.hpp"

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
    private:
        PNetwork mNetwork;
        PI2C mI2C;
};

#endif // PROBOT_HPP
