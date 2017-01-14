#ifndef PQUEUE_HPP
#define PQUEUE_HPP

#include <mutex>
#include <condition_variable>
#include <queue>

template<typename T>
class PQueue
{
    public:

        PQueue() : mRun(true){}

        void push(const T &element)
        {
            std::unique_lock<std::mutex> lock(mMutex);
            mQueue.push(element);
            lock.unlock();
            mCond.notify_one();
        }
        bool pop(T &first)
        {
            std::unique_lock<std::mutex> lock(mMutex);
            while(mQueue.empty() && mRun)
            {
                mCond.wait(lock);
            }
            if(mRun)
            {
                first = mQueue.front();
                mQueue.pop();
            }
            return mRun;

        }

        void stopWaiting()
        {
            mRun=false;
            mCond.notify_all();
        }


    private:
        std::mutex mMutex;
        std::condition_variable mCond;
        std::queue<T> mQueue;

        std::atomic_bool mRun;
};

#endif // PQUEUE_HPP
