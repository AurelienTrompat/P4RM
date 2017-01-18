#ifndef PAGENT_HPP
#define PAGENT_HPP

#include "PThread.hpp"
#include "PMaster.hpp"
#include "PQueue.hpp"
#include "PEvent.hpp"

class PAgent : public PThread
{
    public:
        PAgent();
        ~PAgent();

        void bindMaster(PMaster *master);
        PQueue<PCommand>* getCommandQueue();
    protected:

        virtual void preRun() = 0;
        virtual void run() = 0;
        virtual void postRun() = 0;

        void pushEvent(PEvent &event);
        virtual void handleCommand(const PCommand &command) = 0;

        void setAgent(const Agent agent);

    private:
        void childStart();
        void childStop();
        void pollCommand();
    private:
        std::thread mCommandThread;

        PMaster *mMaster;
        PQueue<PCommand> mCommandQueue;

        Agent mAgent;
};

#endif // PAGENT_HPP
