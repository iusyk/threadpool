
#include "threadpool.h"
#include <future>

using namespace std;

ThreadPool::ThreadPool(size_t size):
        stop{false},
        threads(size)
{
    auto threadFunc= [this]
    {
        for(;;)
        {
            std::function<void()> task;
            {
                std::unique_lock<std::mutex> lock(this->syncMutex);
                this->condition.wait(lock,
                    [this]{ return this->stop || !this->tasks.empty(); });

                if(this->stop && this->tasks.empty())
                    return;
                    
                task = move(this->tasks.front());
                this->tasks.pop();
            };
            task();
            newTaskCondition.notify_one();
        }
    };
    for(auto &thr: threads)
        thr= thread(threadFunc);
}
ThreadPool::~ThreadPool()
{
    { 
        std::unique_lock<std::mutex> lock(syncMutex);
        stop = true;
    }

    condition.notify_all();
    newTaskCondition.notify_all();

    for(auto& thr: threads)
        thr.join();
}
void ThreadPool::push(std::shared_ptr<packaged_task<void()>> task)
{
    {
        unique_lock<mutex> lock(syncMutex);
        if(stop)
            throw runtime_error("ThreadPoll is stopped.");

        tasks.emplace([task](){ (*task)(); });
    }
    condition.notify_one();
    newTaskCondition.notify_one();
}

void ThreadPool::waitAvailability()
{
    std::unique_lock<std::mutex> lock(syncMutex);
    // wait until at least one thread is available
    newTaskCondition.wait(lock,
        [this]{
            return !this->stop && this->tasks.size() < this->threads.size(); 
        }
    );
}
