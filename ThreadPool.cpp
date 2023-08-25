#include "ThreadPool.hpp"
#include <thread>
#include <iostream>

ThreadPool::ThreadPool(size_t threads)
    : stop(false) 
{
    std::cout << "use thread: " << std::thread::hardware_concurrency() << std::endl;

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

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop = true;
    }
    condition.notify_all();
    for(std::thread &worker: workers)
        worker.join();
}

std::future<text> ThreadPool::enqueue(std::function<text()> task) {
    auto wrapped_task = std::make_shared<std::packaged_task<text()>>(task);

    std::future<text> res = wrapped_task->get_future();
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        tasks.emplace([=]() {
            (*wrapped_task)();
        });
    }
    condition.notify_one();
    return res;
}
