#include <cassert>
#include <cmath>
#include <immintrin.h>
#include <thread>
#include <vector>

#include "PiLib.h"

namespace PiLib
{
bool IsSimdThreadPiSupported()
{
	return __builtin_cpu_supports("avx512f");
}

[[gnu::target("avx512f")]] void SimdThunk(const int64_t loopCountStart, const int64_t loopCountEnd,
                                          const double stepScalar, std::atomic<double> &globalSum)
{
	constexpr int LANES = 512 / 8 / sizeof(double);
	const __m512d step = _mm512_set1_pd(stepScalar);
	const __m512d one = _mm512_set1_pd(1);
	const __m512d four = _mm512_set1_pd(4);
	const __m512d lanes = _mm512_set1_pd(LANES);
	const __m512d startOffset = _mm512_set1_pd(static_cast<double>(LANES) * loopCountStart);

	__m512d sum = _mm512_setzero_pd();

	__m512d indexes = _mm512_set_pd(0.5, 1.5, 2.5, 3.5, 4.5, 5.5, 6.5, 7.5);
	indexes = _mm512_add_pd(indexes, startOffset);

	for (int64_t i = loopCountStart; i < loopCountEnd; i++)
	{
		__m512d working = indexes;
		working = _mm512_mul_pd(working, step);
		working = _mm512_fmadd_pd(working, working, one);
		working = _mm512_div_pd(four, working);

		sum = _mm512_add_pd(sum, working);

		indexes = _mm512_add_pd(indexes, lanes);
	}

	globalSum += _mm512_reduce_add_pd(sum);
}

double SimdThreadPi(const int64_t iterations)
{
	return SimdThreadPi(iterations, std::thread::hardware_concurrency());
}

double SimdThreadPi(const int64_t iterations, const std::size_t threadCount)
{
	assert(threadCount > 0);

	constexpr int LANES = 512 / 8 / sizeof(double);
	const int64_t totalLoopCount = std::ceil(iterations / LANES);
	const int64_t perThreadLoopCount = totalLoopCount / threadCount;
	const double stepScalar = 1 / static_cast<double>(totalLoopCount * LANES);

	std::vector<std::thread> threads;
	std::atomic<double> sum{0};

	// Start `threadCount - 1` threads as this thread will do work as well
	threads.reserve(threadCount - 1);
	std::size_t i;
	for (i = 0; i < threadCount - 1; i++)
	{
		threads.emplace_back(SimdThunk, i * perThreadLoopCount, (i + 1) * perThreadLoopCount, stepScalar,
		                     std::ref(sum));
	}

	// Do work on this thread as well. Note that while we round up the number of iterations
	// to be divisible by the number of SIMD elements, we do nothing to ensure it is divisible
	// by the number of threads. For this reason this thread will pick up any of the extra slack
	SimdThunk(i * perThreadLoopCount, totalLoopCount, stepScalar, sum);

	// Join the other threads to ensure the computation is complete
	for (auto &thread : threads)
	{
		thread.join();
	}

	// Be sure to divide by the actual amount of iterations run, not just the requested
	return sum / (totalLoopCount * LANES);
}
} // namespace PiLib
