# Thread Pool

A header-only, C++20 thread pool.

## Example

The following is a parallel sum, where a container of integers is chunked and summed on a thread pool.

```cpp
int ParallelSum(const std::vector<int>& container, ThreadPool& pool)
{
    // Currently setup to run at a minimum 1 sum per thread
    if (container.size() < pool.ThreadCount())
        return 0;

    const auto SumRange = [](auto begin, auto end) {
        return std::accumulate(begin, end, 0);
    };

    // Determine the amount of work to do on each thread
    const std::ptrdiff_t chunkSize = container.size() / pool.ThreadCount();

    std::vector<std::future<int>> results;
    for (auto i = 1; i <= pool.ThreadCount(); ++i)
    {
        auto begin = std::next(container.cbegin(), (i - 1) * chunkSize);
        auto end = std::next(container.cbegin(), i * chunkSize);

        // Push each accumulation task into the thread pool
        results.push_back(pool.Run(SumRange, begin, end));
    }

    // Add the results computed on the thread pool
    return std::accumulate(results.begin(), results.end(), 0, [](int aggregate, std::future<int>& result) {
        return aggregate + result.get();
    });
}
```