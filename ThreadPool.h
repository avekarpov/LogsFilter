#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <vector>
#include <queue>

#include <thread>
#include <mutex>
#include <functional>
#include <future>

class ThreadPool {
public:
    explicit ThreadPool(unsigned numberThreads = std::thread::hardware_concurrency());

    template<typename F, typename ...Args>
    auto enqueue(F &&taskFunction, Args &&...args) -> std::future<typename std::invoke_result<F, Args ...>::type>
    {
        using returnType = typename std::invoke_result<F, Args ...>::type;

        auto task = std::make_shared<std::packaged_task<returnType()>>(std::bind(std::forward<F>(taskFunction), std::forward<Args>(args)...));
        auto future = (*task).get_future();

        {
            std::unique_lock<std::mutex> lock(_tasksMutex);

            _tasks.push([=]() { (*task)(); });
        }

        _waiterForNewTask.notify_one();

        return future;
    }

    ~ThreadPool();

private:
    std::vector<std::thread> _workers;

    std::queue<std::function<void()>> _tasks;
    std::mutex _tasksMutex;

    std::condition_variable _waiterForNewTask;

    bool _isStop;
};

#endif //THREADPOOL_H
