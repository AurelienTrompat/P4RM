#ifndef PAGENT_HPP
#define PAGENT_HPP

#include <thread>
#include <atomic>
#include "PQueue.hpp"
#include "PCommand.hpp"


class PAgent
{
    public:
        PAgent();
        ~PAgent();

        void start();
        void stop();
    protected:
        void execute();
        virtual void preRun() = 0;
        virtual void run() = 0;
        virtual void postRun() = 0;

        bool getRunState() const;

    private:
        std::thread mThread;
        std::atomic_bool mRun;

        PQueue<PCommand> mQueue;
};

#endif // PAGENT_HPP
