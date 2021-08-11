#include "ThreadPool.h"

ThreadPool::ThreadPool(unsigned int numberThreads)  : _isStop(false)
{
    for (unsigned i = 0; i < numberThreads; ++i)
    {
        this->_workers.emplace_back([&]()
        {
            std::function <void()> task;

            while (!_isStop)
            {
                {
                    std::unique_lock<std::mutex> lock(_tasksMutex);

                    if (_tasks.empty())
                    {
                        _waiterForNewTask.wait(lock);

                        if (_isStop)
                        {
                            break;
                        }
                    }

                    task = _tasks.front();
                    _tasks.pop();
                }

                task();
            }
        });
    }
}

ThreadPool::~ThreadPool()
{
    _isStop = true;

    _waiterForNewTask.notify_all();

    for (auto &worker : _workers)
    {
        worker.join();
    }
}