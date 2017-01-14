#ifndef PTHREAD_HPP
#define PTHREAD_HPP

#include <thread>
#include <atomic>


class PThread
{
    public:
        PThread();
        ~PThread();

        void start();
        void stop();

    protected:
        virtual void childStart();
        virtual void childStop();

        virtual void preRun() = 0;
        virtual void run() = 0;
        virtual void postRun() = 0;

        bool getRunState() const;
    private:
        void execute();
    private:
        std::thread mRunThread;
        std::atomic_bool mRun;
};

#endif // PTHREAD_HPP
