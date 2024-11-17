#include <numeric>
#include <omp.h>
#include <vector>

#include "PiLib.h"

namespace PiLib
{
double NaiveOmpPi(const int64_t iterations)
{
	return NaiveOmpPi(iterations, omp_get_max_threads());
}

double NaiveOmpPi(const int64_t iterations, const std::size_t threadCount)
{
	const double step = 1 / static_cast<double>(iterations);
	std::vector<double> sum (threadCount);

	omp_set_num_threads(threadCount);
	#pragma omp parallel
	{
		const auto id = omp_get_thread_num();
		const auto threads = omp_get_num_threads();

		double innerSum = 0;
		for (int64_t i = id; i < iterations; i += threads)
		{
			const double x = (i + 0.5) * step;
			innerSum += 4.0 / (1.0 + x * x);
		}

		sum[id] = innerSum;
	}

	double sumReduced = std::accumulate(sum.cbegin(), sum.cend(), 0.0);
	return step * sumReduced;
}
} // namespace PiLib
