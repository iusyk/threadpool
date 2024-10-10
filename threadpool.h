#ifndef __THREADPOOL_H__
#define __THREADPOOL_H__

#include <string>
#include <queue>
#include <vector>
#include <functional>
#include <thread>
#include <condition_variable>
#include <future>

class ThreadPool final
{
public:
    explicit ThreadPool(std::size_t size);
    ~ThreadPool();
    void push(std::shared_ptr<std::packaged_task<void()>> task);
    void waitAvailability();
private:
    bool stop;
    std::condition_variable condition;
    std::mutex syncMutex;
    std::condition_variable newTaskCondition;
    std::vector<std::thread> threads;
    std::queue<std::function<void()>> tasks;
};

#endif