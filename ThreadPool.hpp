#ifndef THREADPOOL_HPP
#define THREADPOOL_HPP

#include <vector>
#include <queue>
#include <functional>
#include <thread>
#include <condition_variable>
#include <future>
#include "text.hpp"

class ThreadPool {
public:
    ThreadPool(size_t threads);
    ~ThreadPool();
    
    std::future<text> enqueue(std::function<text()> task);


private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;

    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop;
};

#endif // THREADPOOL_HPP
