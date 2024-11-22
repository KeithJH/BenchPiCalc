#include <numbers>

#include "PiLib.h"

namespace PiLib
{
constexpr double SerialPiInternal(const std::size_t iterations)
{
	double sum = 0;
	double step = 1 / static_cast<double>(iterations);

	for (std::size_t i = 0; i < iterations; i++)
	{
		const double di = static_cast<double>(i) * 1.0;
		const double x = (di + 0.5) * step;

		sum += 4.0 / (1.0 + x * x);
	}

	return step * sum;
}

double SerialPi(const std::size_t iterations)
{
	return SerialPiInternal(iterations);
}

// Basic compile-time sanity checks
static_assert(SerialPiInternal(10) >= std::numbers::pi - 0.01);
static_assert(SerialPiInternal(10) <= std::numbers::pi + 0.01);
} // namespace PiLib
