#include "PAgent.hpp"


PAgent::PAgent() : mCommandThread(), mMaster(nullptr), mCommandQueue(){}
PAgent::~PAgent(){}

void PAgent::childStart()
{
    mCommandThread = std::thread(&PAgent::pollCommand, this);
}

void PAgent::childStop()
{
    PCommand command;
    command.mType = PCommand::Type::Quit;
    mCommandQueue.push(command);
    mCommandThread.join();
}
void PAgent::bindMaster(PMaster *master)
{
    mMaster = master;
}

void PAgent::pushEvent(const PEvent &event)
{
    if(mMaster!= nullptr)
        mMaster->putEvent(event);
}
void PAgent::pollCommand()
{
    PCommand command;
    while(getRunState())
    {
        mCommandQueue.pop(command);
        if(command.mType != PCommand::Type::Quit)
            handleCommand(command);
        else
            while(getRunState());
    }
}
PQueue<PCommand>* PAgent::getCommandQueue()
{
    return &mCommandQueue;
}
