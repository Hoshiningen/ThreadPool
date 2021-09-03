#include "ThreadPool.h"

#include <array>
#include <execution>
#include <numeric>
#include <span>

#include <benchmark/benchmark.h>

static std::vector<int> g_integers = []()
{
    std::vector<int> items;
    items.resize(100'000'000, 1);
    
    return items;
}();

static const std::array<std::span<const int>, 6> spans
{
    std::span{g_integers.cbegin(), std::next(g_integers.cbegin(), 1'000)},
    std::span{g_integers.cbegin(), std::next(g_integers.cbegin(), 10'000)},
    std::span{g_integers.cbegin(), std::next(g_integers.cbegin(), 100'000)},
    std::span{g_integers.cbegin(), std::next(g_integers.cbegin(), 1'000'000)},
    std::span{g_integers.cbegin(), std::next(g_integers.cbegin(), 10'000'000)},
    std::span{g_integers.cbegin(), g_integers.cend()}
};

template<typename T>
int ParallelSum(const std::span<const T>& span, ThreadPool& pool)
{
    if (span.size() < pool.ThreadCount())
        return 0;

    const auto SumRange = [](auto begin, auto end) {
        return std::accumulate(begin, end, 0);
    };

    const std::ptrdiff_t chunkSize = span.size() / pool.ThreadCount();

    std::vector<std::future<T>> results;
    for (auto i = 1; i <= pool.ThreadCount(); ++i)
    {
        results.push_back(pool.Run(SumRange,
            std::next(span.begin(), (i - 1) * chunkSize),
            std::next(span.begin(), i * chunkSize)));
    }

    const auto result = std::accumulate(results.begin(), results.end(), 0, [](T aggregate, std::future<T>& result) {
        return aggregate + result.get();
    });

    return result;
}

//==================================================================================
// Throughput benchmarks
//==================================================================================

static void ThreadPoolTP(benchmark::State& state)
{
    ThreadPool pool;
    const auto& span = spans.at(state.range(0));

    for (auto _ : state)
    {
        state.counters["sum"] = ParallelSum(span, pool);
    }

    state.SetItemsProcessed(state.iterations() * span.size());
}

static void ReduceTP(benchmark::State& state)
{
    const auto& span = spans.at(state.range(0));
    for (auto _ : state)
    {
        state.counters["sum"] = std::reduce(std::execution::par_unseq, span.begin(), span.end());
    }

    state.SetItemsProcessed(state.iterations() * span.size());
}

static void AccumulateTP(benchmark::State& state)
{
    const auto& span = spans.at(state.range(0));
    for (auto _ : state)
    {
        state.counters["sum"] = std::accumulate(span.begin(), span.end(), 0);
    }

    state.SetItemsProcessed(state.iterations() * span.size());
}

BENCHMARK(ThreadPoolTP)->DenseRange(0, 5)->UseRealTime();
BENCHMARK(ReduceTP)->DenseRange(0, 5)->UseRealTime();
BENCHMARK(AccumulateTP)->DenseRange(0, 5)->UseRealTime();

BENCHMARK_MAIN();