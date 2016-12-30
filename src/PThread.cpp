#include "PThread.hpp"

PThread::PThread() : mRunThread(), mRun(false)
{
    //ctor
}

PThread::~PThread()
{
    stop();
}
void PThread::start()
{
    mRun = true;
    mRunThread = std::thread(&PThread::execute, this);
    childStart();
}

void PThread::stop()
{
    if(mRun)
    {
        mRun = false;
        childStop();
        mRunThread.join();
    }
}

bool PThread::getRunState() const
{
    return mRun;
}

void PThread::execute()
{
    preRun();
    while(mRun) run();
    postRun();
}
void PThread::childStart()
{

}

void PThread::childStop()
{

}
