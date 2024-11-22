#include <cstddef>
#include <omp.h>

#include "PiLib.h"

namespace PiLib
{
double AtomicOmpPi(const std::size_t iterations)
{
	return AtomicOmpPi(iterations, static_cast<std::size_t>(omp_get_max_threads()));
}

double AtomicOmpPi(const std::size_t iterations, const std::size_t threadCount)
{
	const double step = 1 / static_cast<double>(iterations);
	double sum{0};

	omp_set_num_threads(static_cast<int>(threadCount));
#pragma omp parallel
	{
		const auto id = static_cast<std::size_t>(omp_get_thread_num());
		const auto threads = static_cast<std::size_t>(omp_get_num_threads());

		double innerSum = 0;
		for (std::size_t i = id; i < iterations; i += threads)
		{
			const double x = (static_cast<double>(i) + 0.5) * step;
			innerSum += 4.0 / (1.0 + x * x);
		}

#pragma omp atomic
		sum += innerSum;
	}

	return step * sum;
}
} // namespace PiLib
