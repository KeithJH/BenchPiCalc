#include <omp.h>

#include "PiLib.h"

namespace PiLib
{
double AtomicOmpPi(const int64_t iterations)
{
	return AtomicOmpPi(iterations, omp_get_max_threads());
}

double AtomicOmpPi(const int64_t iterations, const std::size_t threadCount)
{
	const double step = 1 / static_cast<double>(iterations);
	double sum{0};

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

#pragma omp atomic
		sum += innerSum;
	}

	return step * sum;
}
} // namespace PiLib
