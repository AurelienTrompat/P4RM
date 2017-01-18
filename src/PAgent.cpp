#include "PAgent.hpp"


PAgent::PAgent() : mCommandThread(), mMaster(nullptr), mCommandQueue(){}
PAgent::~PAgent(){}

void PAgent::childStart()
{
    mCommandThread = std::thread(&PAgent::pollCommand, this);
}

void PAgent::childStop()
{
    mCommandQueue.stopWaiting();
    mCommandThread.join();
}
void PAgent::bindMaster(PMaster *master)
{
    mMaster = master;
}

void PAgent::pushEvent(PEvent &event)
{
    event.mAgent=mAgent;
    if(mMaster!= nullptr)
        mMaster->putEvent(event);
}
void PAgent::pollCommand()
{
    PCommand command;
    while(getRunState())
    {
        if(mCommandQueue.pop(command))
            handleCommand(command);
    }
}
PQueue<PCommand>* PAgent::getCommandQueue()
{
    return &mCommandQueue;
}

void PAgent::setAgent(const Agent agent)
{
    mAgent = agent;
}
