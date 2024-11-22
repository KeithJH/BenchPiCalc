#include <numeric>
#include <omp.h>
#include <vector>

#include "PiLib.h"

namespace PiLib
{
double FalseSharingOmpPi(const std::size_t iterations)
{
	return FalseSharingOmpPi(iterations, static_cast<std::size_t>(omp_get_max_threads()));
}

double FalseSharingOmpPi(const std::size_t iterations, const std::size_t threadCount)
{
	const double step = 1 / static_cast<double>(iterations);
	std::vector<double> sum (threadCount);

	omp_set_num_threads(static_cast<int>(threadCount));
	#pragma omp parallel
	{
		// Volatile is used to prevent optimizing the memory access of `sum` away inside
		// the below for loop. It's not entirely necesary with all compilers or all
		// optimization levels.
		volatile auto id = static_cast<std::size_t>(omp_get_thread_num());
		const auto threads = static_cast<std::size_t>(omp_get_num_threads());

		for (std::size_t i = id; i < iterations; i += threads)
		{
			const double x = (static_cast<double>(i) + 0.5) * step;

			// This should cause false sharing as all elements of the vector are on
			// the same cache line, as long as the memory access is actually generated
			sum[id] += 4.0 / (1.0 + x * x);
		}
	}

	double sumReduced = std::accumulate(sum.cbegin(), sum.cend(), 0.0);
	return step * sumReduced;
}
} // namespace PiLib
