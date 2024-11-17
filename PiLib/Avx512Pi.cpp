#include <immintrin.h>

#include "PiLib.h"

namespace PiLib
{
bool IsAvx512PiSupported()
{
	return __builtin_cpu_supports("avx512f");
}

__attribute__((target("avx512f"))) double Avx512Pi(const int64_t iterations)
{
	// How many iteraitons are we doing at once in a single vectorized command
	constexpr int LANES = 512 / 8 / sizeof(double);

	// We will do at least `iterations`, but may do more to avoid
	// doing any scalar "tail" computations
	const int64_t loopCount = (iterations / LANES) + (iterations % LANES > 0 ? 1 : 0);
	const double stepScalar = 1 / static_cast<double>(loopCount * LANES);

	// Broadcast reused scalar values for vector math
	const __m512d step = _mm512_set1_pd(stepScalar);
	const __m512d one = _mm512_set1_pd(1);
	const __m512d four = _mm512_set1_pd(4);
	const __m512d lanes = _mm512_set1_pd(LANES);

	__m512d sum = _mm512_setzero_pd();

	// `indexes` is effectively our vectorized `i`
	// To account for doing mid-point sums we start the indexes of our calculations at +0.5
	__m512d indexes = _mm512_set_pd(0.5, 1.5, 2.5, 3.5, 4.5, 5.5, 6.5, 7.5);
	for (int64_t i = 0; i < loopCount; i++)
	{
		// x = (i + 0.5) * step
		// sum += 4.0 / (1.0 + x * x)

		__m512d working = indexes;                        // x = i + 0.5
		working = _mm512_mul_pd(working, step);           // x = x * step
		working = _mm512_fmadd_pd(working, working, one); // denom = (x * x) + 1.0
		working = _mm512_div_pd(four, working);           // temp = 4.0 / denom

		sum = _mm512_add_pd(sum, working); // sum += temp

		indexes = _mm512_add_pd(indexes, lanes); // i++
	}

	// convert to scalar result
	return stepScalar * _mm512_reduce_add_pd(sum);
}
} // namespace PiLib
