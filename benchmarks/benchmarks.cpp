#include "ThreadPool.h"

#include <benchmark/benchmark.h>

std::vector<bool> ThreadFuncMulti(std::size_t amount)
{
    std::vector<bool> things;

    for (auto i = 0; i < amount; ++i)
        things.push_back(i % 2 == 0);

    return things;
}

static void ThreadPoolTP(benchmark::State& state)
{
    ThreadPool pool{ static_cast<unsigned int>(state.range(0)) };

    std::size_t itemsProcessed = 0;
    for (auto _ : state)
    {
        state.PauseTiming();
        std::vector<std::future<std::vector<bool>>> futures;
        state.ResumeTiming();

        for (std::size_t i = 0; i < state.range(1); ++i)
            futures.push_back(pool.Run(ThreadFuncMulti, state.range(2)));

        for (auto& future : futures) {
            auto vector = future.get();
            itemsProcessed += vector.size();
        }
    }

    state.SetItemsProcessed(itemsProcessed);
}

static void LaunchDefaultTP(benchmark::State& state)
{
    ThreadPool pool{ static_cast<unsigned int>(state.range(0)) };

    std::size_t itemsProcessed = 0;
    for (auto _ : state)
    {
        state.PauseTiming();
        std::vector<std::future<std::vector<bool>>> futures;
        state.ResumeTiming();

        for (std::size_t i = 0; i < state.range(1); ++i)
            futures.push_back(std::async(ThreadFuncMulti, state.range(2)));

        for (auto& future : futures) {
            future.get();
            ++itemsProcessed;
        }
    }

    state.SetItemsProcessed(itemsProcessed);
}

static void LaunchAsyncTP(benchmark::State& state)
{
    ThreadPool pool{ static_cast<unsigned int>(state.range(0)) };

    std::size_t itemsProcessed = 0;
    for (auto _ : state)
    {
        state.PauseTiming();
        std::vector<std::future<std::vector<bool>>> futures;
        state.ResumeTiming();

        for (std::size_t i = 0; i < state.range(1); ++i)
            futures.push_back(std::async(std::launch::async, ThreadFuncMulti, state.range(2)));

        for (auto& future : futures) {
            future.get();
            ++itemsProcessed;
        }
    }

    state.SetItemsProcessed(itemsProcessed);
}

static void LaunchDeferredTP(benchmark::State& state)
{
    ThreadPool pool{ static_cast<unsigned int>(state.range(0)) };

    std::size_t itemsProcessed = 0;
    for (auto _ : state)
    {
        state.PauseTiming();
        std::vector<std::future<std::vector<bool>>> futures;
        state.ResumeTiming();

        for (std::size_t i = 0; i < state.range(1); ++i)
            futures.push_back(std::async(std::launch::deferred, ThreadFuncMulti, state.range(2)));

        for (auto& future : futures) {
            future.get();
            ++itemsProcessed;
        }
    }

    state.SetItemsProcessed(itemsProcessed);
}

BENCHMARK(ThreadPoolTP)->ArgsProduct({
    {1, 2, 3, 4},
    {1, 2, 3, 4, 5, 6, 7, 8, 9, 10},
    {10000}
});

BENCHMARK(LaunchAsyncTP)->ArgsProduct({
    {1, 2, 3, 4},
    {1, 2, 3, 4, 5, 6, 7, 8, 9, 10},
    {10000}
});

BENCHMARK(LaunchDeferredTP)->ArgsProduct({
    {1, 2, 3, 4},
    {1, 2, 3, 4, 5, 6, 7, 8, 9, 10},
    {10000}
});

BENCHMARK(LaunchDefaultTP)->ArgsProduct({
    {1, 2, 3, 4},
    {1, 2, 3, 4, 5, 6, 7, 8, 9, 10},
    {10000}
});

BENCHMARK_MAIN();