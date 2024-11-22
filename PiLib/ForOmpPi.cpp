#include <omp.h>

#include "PiLib.h"

namespace PiLib
{
double ForOmpPi(const std::size_t iterations)
{
	return ForOmpPi(iterations, static_cast<std::size_t>(omp_get_max_threads()));
}

double ForOmpPi(const std::size_t iterations, const std::size_t threadCount)
{
	const double step = 1 / static_cast<double>(iterations);
	double sum{0};

	omp_set_num_threads(static_cast<int>(threadCount));
#pragma omp parallel for reduction(+ : sum)
	for (std::size_t i = 0; i < iterations; i++)
	{
		const double x = (static_cast<double>(i) + 0.5) * step;
		sum += 4.0 / (1.0 + x * x);
	}

	return step * sum;
}
} // namespace PiLib
