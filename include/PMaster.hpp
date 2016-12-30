#ifndef PMASTER_HPP
#define PMASTER_HPP

#include <memory>
#include <map>
#include "PThread.hpp"
#include "PQueue.hpp"
#include "PCommand.hpp"
#include "PEvent.hpp"

class PMaster : public PThread
{
    public:
        PMaster();
        ~PMaster();

        void putEvent(const PEvent event);

    protected:
        virtual void preRun() = 0;
        virtual void run() = 0;
        virtual void postRun() = 0;

        void pushCommand(const PCommand command);
        virtual void handleEvent(const PEvent &event) = 0;

        void bindCommandeQueue(const PCommand::Agent agent, PQueue<PCommand> *commandQueue);
    private:
        void childStart();
        void childStop();
        void pollEvent();

    private:
        std::thread mEventThread;

        PQueue<PEvent> mEventQueue;
        std::map<PCommand::Agent, PQueue<PCommand>*> mCommandQueueMap;


};

#endif // PMASTER_HPP
