#include "ThreadPool.hpp"

ThreadPool::ThreadPool(size_t threads): stop(false) {
    for(size_t i = 0; i < threads; ++i)
        workers.emplace_back(
            [this]
            {
                for(;;)
                {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(this->queue_mutex);
                        this->condition.wait(lock,
                            [this]{ return this->stop || !this->tasks.empty(); });
                        if(this->stop && this->tasks.empty())
                            return;
                        task = std::move(this->tasks.front());
                        this->tasks.pop();
                    }
                    task();
                }
            }
        );
}

// add new work item to the pool
std::future<void> ThreadPool::enqueue(std::function<void()> task)
{
    auto future = std::make_shared<std::promise<void>>();
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        tasks.emplace([task, future]() {
            task();
            future->set_value();
        });
    }
    condition.notify_one();
    return future->get_future();
}

// the destructor joins all threads
ThreadPool::~ThreadPool()
{
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop = true;
    }
    condition.notify_all();
    for(std::thread &worker: workers)
        worker.join();
}
