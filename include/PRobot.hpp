#ifndef PROBOT_HPP
#define PROBOT_HPP

#include <iostream>
#include "PMaster.hpp"
#include "PNetwork.hpp"


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
};

#endif // PROBOT_HPP
