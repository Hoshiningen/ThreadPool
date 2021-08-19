#pragma once

#include <algorithm>
#include <concepts>
#include <condition_variable>
#include <functional>
#include <future>
#include <queue>
#include <ranges>
#include <stdexcept>
#include <thread>
#include <type_traits>

class ThreadPool
{
public:

    ThreadPool()
        : m_threads(std::thread::hardware_concurrency())
    {
        if (m_threads.size() == 0)
            throw std::invalid_argument("Cannot construct a thread pool with 0 threads.");

        std::ranges::generate(m_threads, [this]() {
            return std::jthread{ std::bind_front(&ThreadPool::ThreadFunc, this) };
        });
    }

    explicit ThreadPool(unsigned int numThreads)
        : m_threads(numThreads)
    {
        if (m_threads.size() == 0)
            throw std::invalid_argument("Cannot construct a thread pool with 0 threads.");

        std::ranges::generate(m_threads, [this]() {
            return std::jthread{ std::bind_front(&ThreadPool::ThreadFunc, this) };
        });
    }

    ~ThreadPool()
    {
        std::ranges::for_each(m_threads, [](std::jthread& thread) {
            thread.request_stop();
            thread.join();
        });
    }

    // Disallow copying
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    // Disallow moving
    ThreadPool(ThreadPool&&) = delete;
    ThreadPool& operator=(ThreadPool&&) = delete;

    template<typename F, typename...Args>
        requires std::invocable<F, Args...>
    [[nodiscard]] auto Run(F&& func, Args&&... args)
    {
        using R = std::invoke_result_t<std::decay_t<F>, std::decay_t<Args>...>;

        auto pTask = std::make_shared<std::packaged_task<R(std::decay_t<Args>...)>>(func);
        auto future = pTask->get_future();

        std::scoped_lock<std::mutex> lock{ m_queueMut };

        m_taskQueue.emplace([pTask = std::move(pTask), ...args = std::forward<Args>(args)]() {
            std::invoke(*pTask, args...);
        });

        m_cv.notify_one();

        return future;
    }

    std::size_t ThreadCount() const { return m_threads.size(); }

private:

    void ThreadFunc(const std::stop_token& stopToken)
    {
        while (true)
        {
            std::unique_lock<std::mutex> lock{ m_queueMut };
            if (m_cv.wait(lock, stopToken, [this]() { return !m_taskQueue.empty(); }))
            {
                // Execute the task at the front of the queue, then remove it
                m_taskQueue.front()();
                m_taskQueue.pop();
            }
            else
            {
                // Break out of the loop if we can no longer wait
                if (stopToken.stop_requested())
                    break;
            }
        }
    }

    std::condition_variable_any m_cv;
    std::mutex m_queueMut;

    std::vector<std::jthread> m_threads;
    std::queue<std::function<void()>> m_taskQueue;
};