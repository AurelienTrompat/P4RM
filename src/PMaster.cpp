#include "PMaster.hpp"

using namespace std;

PMaster::PMaster() : mEventThread(), mEventQueue(), mCommandQueueMap(){}
PMaster::~PMaster(){}

void PMaster::childStart()
{
    mEventThread = std::thread(&PMaster::pollEvent, this);
}

void PMaster::childStop()
{
    mEventQueue.stopWaiting();
    mEventThread.join();
}
void PMaster::putEvent(const PEvent event)
{
    mEventQueue.push(event);
}

void PMaster::bindCommandeQueue(const Agent agent, PQueue<PCommand> *commandQueue)
{
    const auto found = mCommandQueueMap.find(agent);
    if(found == mCommandQueueMap.end())
    {
        mCommandQueueMap.insert(std::make_pair(agent, commandQueue));
    }
}


void PMaster::pushCommand(const PCommand command)
{
    const auto found = mCommandQueueMap.find(command.mAgent);
    if(found != mCommandQueueMap.end())
    {
        (mCommandQueueMap.at(command.mAgent))->push(command);
    }
}

void PMaster::pollEvent()
{
    PEvent event;
    while(getRunState())
    {
        if(mEventQueue.pop(event))
            handleEvent(event);
    }
}

