#ifndef PQUEUE_HPP
#define PQUEUE_HPP

#include <mutex>
#include <condition_variable>
#include <queue>

template<typename T>
class PQueue
{
    public:

        void push(const T &element)
        {
            std::unique_lock<std::mutex> lock(mMutex);
            mQueue.push(element);
            lock.unlock();
            mCond.notify_one();
        }
        void pop(T &first)
        {
            std::unique_lock<std::mutex> lock(mMutex);
            while(mQueue.empty())
            {
                mCond.wait(lock);
            }
            first = mQueue.front();
            mQueue.pop();
        }


    private:
        std::mutex mMutex;
        std::condition_variable mCond;
        std::queue<T> mQueue;
};

#endif // PQUEUE_HPP
