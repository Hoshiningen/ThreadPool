#include "ThreadPool.h"

#include <array>
#include <execution>
#include <numeric>

#include <benchmark/benchmark.h>

static const std::array<std::vector<int>, 4> containers
{
    []()
    {
        std::vector<int> items;
        items.resize(100'000, 1);

        return items;
    }(),

    []()
    {
        std::vector<int> items;
        items.resize(1'000'000, 1);

        return items;
    }(),

    []()
    {
        std::vector<int> items;
        items.resize(10'000'000, 1);

        return items;
    }(),

    []()
    {
        std::vector<int> items;
        items.resize(100'000'000, 1);

        return items;
    }()
};

int ParallelSum(const std::vector<int>& container, ThreadPool& pool)
{
    if (container.size() < pool.ThreadCount())
        return 0;

    const auto SumRange = [](auto begin, auto end) {
        return std::accumulate(begin, end, 0);
    };

    const std::ptrdiff_t chunkSize = container.size() / pool.ThreadCount();

    std::vector<std::future<int>> results;
    for (auto i = 1; i <= pool.ThreadCount(); ++i)
    {
        auto begin = std::next(container.cbegin(), (i - 1) * chunkSize);
        auto end = std::next(container.cbegin(), i * chunkSize);

        results.push_back(pool.Run(SumRange, begin, end));
    }

    return std::accumulate(results.begin(), results.end(), 0, [](int aggregate, std::future<int>& result) {
        return aggregate + result.get();
    });
}

std::vector<bool> ThreadFuncMulti(std::size_t amount)
{
    std::vector<bool> things;

    for (auto i = 0; i < amount; ++i)
        things.push_back(i % 2 == 0);

    return things;
}

static void ParallelSumBM(benchmark::State& state)
{
    ThreadPool pool;
    std::vector<int> items;

    for (auto _ : state)
    {
        benchmark::DoNotOptimize(ParallelSum(containers.at(state.range(0)), pool));
    }
}

static void ReduceBM(benchmark::State& state)
{
    const std::vector<int>& items = containers.at(state.range(0));
    for (auto _ : state)
    {
        benchmark::DoNotOptimize(std::reduce(std::execution::par_unseq, items.cbegin(), items.cend()));
    }
}

static void AccumulateBM(benchmark::State& state)
{
    const std::vector<int>& items = containers.at(state.range(0));
    for (auto _ : state)
    {
        benchmark::DoNotOptimize(std::accumulate(items.cbegin(), items.cend(), 0));
    }
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

BENCHMARK(ParallelSumBM)->Args({0, 1, 2, 3});
BENCHMARK(ReduceBM)->Args({0, 1, 2, 3});
BENCHMARK(AccumulateBM)->Args({0, 1, 2, 3});

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