#include <immintrin.h>

#include "PiLib.h"

namespace PiLib
{
double SSE2Pi(const int64_t iterations)
{
	// How many iteraitons are we doing at once in a single vectorized command
	constexpr int LANES = 128 / 8 / sizeof(double);

	// We will do at least `iterations`, but may do more to avoid
	// doing any scalar "tail" computations
	const int64_t loopCount = (iterations / LANES) + (iterations % LANES > 0 ? 1 : 0);
	const double stepScalar = 1 / static_cast<double>(loopCount * LANES);

	// Broadcast reused scalar values for vector math
	const __m128d step = _mm_set1_pd(stepScalar);
	const __m128d one = _mm_set1_pd(1);
	const __m128d four = _mm_set1_pd(4);
	const __m128d lanes = _mm_set1_pd(LANES);

	//double sumScalar = 0;
	__m128d sum = _mm_setzero_pd();

	// `indexes` is effectively our vectorized `i`
	// To account for doing mid-point sums we start the indexes of our calculations at +0.5
	__m128d indexes = _mm_set_pd(0.5, 1.5);
	for (int64_t i = 0; i < loopCount; i++)
	{
		// x = (i + 0.5) * step
		// sum += 4.0 / (1.0 + x * x)

		__m128d working = indexes;              // x = i + 0.5
		working = _mm_mul_pd(working, step);    // x = x * step
		working = _mm_mul_pd(working, working); // x^2 = x * x
		working = _mm_add_pd(working, one);     // denom = 1.0 + x^2
		working = _mm_div_pd(four, working);    // temp = 4.0 / denom

		sum = _mm_add_pd(sum, working);			// sum += temp

		indexes = _mm_add_pd(indexes, lanes);	// i++
	}

	// result = step * sum;
	sum = _mm_mul_pd(sum, step);	

	// convert to scalar result
	double high;
	_mm_storeh_pd(&high, sum);
	double low = _mm_cvtsd_f64(sum);

	return high + low;
}
} // namespace PiLib
