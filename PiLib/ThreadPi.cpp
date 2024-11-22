#include <cassert>
#include <cstddef>
#include <thread>
#include <vector>

#include "PiLib.h"

namespace PiLib
{
void Thunk(const std::size_t iterations, std::size_t threadId, std::size_t threadCount, std::atomic<double> &globalSum)
{
	double localSum{0};
	double step = 1 / static_cast<double>(iterations);

	// Split the work between threads by having a stride of `threadCount` and starting at
	// `threadId`. Since there's no memory access we don't have to worry about any cache
	// locality issues or anything.
	for (std::size_t i = threadId; i < iterations; i += threadCount)
	{
		const double x = (static_cast<double>(i) + 0.5) * step;
		localSum += 4.0 / (1.0 + x * x);
	}

	globalSum += localSum;
}

double ThreadPi(const std::size_t iterations)
{
	return ThreadPi(iterations, std::thread::hardware_concurrency());
}

double ThreadPi(const std::size_t iterations, const std::size_t threadCount)
{
	assert(threadCount > 0);

	std::vector<std::thread> threads;
	std::atomic<double> sum{0};

	// Start `threadCount - 1` threads as this thread will do work as well
	threads.reserve(threadCount - 1);
	for (std::size_t i = 1; i < threadCount; i++)
	{
		threads.emplace_back(Thunk, iterations, i, threadCount, std::ref(sum));
	}

	// Do work on this thread as well
	Thunk(iterations, 0, threadCount, sum);

	// Join the other threads to ensure the computation is complete
	for (auto &thread : threads)
	{
		thread.join();
	}

	return sum / static_cast<double>(iterations);
}
} // namespace PiLib
