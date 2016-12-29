#include "PAgent.hpp"


PAgent::PAgent() : mThread(), mRun(false), mQueue(){}
PAgent::~PAgent()
{
    stop();
}

void PAgent::start()
{
    mRun = true;
    mThread = std::thread(&PAgent::execute, this);
}

void PAgent::stop()
{
    if(mRun)
    {
        mRun = false;
        mThread.join();
    }
}

void PAgent::execute()
{
    preRun();
    while(mRun) run();
    postRun();
}
bool PAgent::getRunState() const
{
    return mRun;
}
