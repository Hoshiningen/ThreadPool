#include "ThreadPool.h"

#include <array>
#include <iostream>
#include <future>
#include <syncstream>
#include <vector>

using namespace std::chrono_literals;

std::vector<int> MakeThings(std::size_t max)
{
    std::osyncstream{ std::cout } << "Making things on thread: " << std::this_thread::get_id() << "\n";

    std::vector<int> things;
    for (auto i = 0; i < max; ++i)
        things.push_back(i);

    return things;
}

void Foobar()
{
    std::osyncstream{ std::cout } << "Foobar on thread: " << std::this_thread::get_id() << "\n";
}

int main()
{
    ThreadPool pool;
    std::vector<int> things;
    
    std::array<std::future<std::vector<int>>, 5> futures
    {
        pool.Run(MakeThings, 2'500'000),
        pool.Run(MakeThings, 2'500'000),
        pool.Run(MakeThings, 2'500'000),
        pool.Run(MakeThings, 2'500'000),
        pool.Run(MakeThings, 2'500'000),
    };
    
    pool.Run(Foobar);

    for (auto& future : futures)
    {
        const std::vector<int> result = future.get();
        things.insert(std::end(things), std::cbegin(result), std::cend(result));
    }
}