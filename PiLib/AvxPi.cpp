#include <immintrin.h>

#include "PiLib.h"

namespace PiLib
{
bool IsAvxPiSupported()
{
	return __builtin_cpu_supports("avx") && __builtin_cpu_supports("fma");
}

__attribute__((target("avx"), target("fma"))) double AvxPi(const int64_t iterations)
{
	// How many iteraitons are we doing at once in a single vectorized command
	constexpr int LANES = 256 / 8 / sizeof(double);

	// We will do at least `iterations`, but may do more to avoid
	// doing any scalar "tail" computations
	const int64_t loopCount = (iterations / LANES) + (iterations % LANES != 0);
	const double stepScalar = 1 / static_cast<double>(loopCount * LANES);

	// Broadcast reused scalar values for vector math
	const __m256d step = _mm256_set1_pd(stepScalar);
	const __m256d one = _mm256_set1_pd(1);
	const __m256d four = _mm256_set1_pd(4);
	const __m256d lanes = four;

	__m256d sum = _mm256_setzero_pd();

	// `indexes` is effectively our vectorized `i`
	// To account for doing mid-point sums we start the indexes of our calculations at +0.5
	__m256d indexes = _mm256_set_pd(0.5, 1.5, 2.5, 3.5);
	for (int64_t i = 0; i < loopCount; i++)
	{
		// x = (i + 0.5) * step
		// sum += 4.0 / (1.0 + x * x)

		__m256d working = indexes;                        // x = i + 0.5
		working = _mm256_mul_pd(working, step);           // x = x * step
		working = _mm256_fmadd_pd(working, working, one); // denom = (x * x) + 1.0
		working = _mm256_div_pd(four, working);           // temp = 4.0 / denom

		sum = _mm256_add_pd(sum, working); // sum += temp

		indexes = _mm256_add_pd(indexes, lanes); // i++
	}

	// Reduce one step
	__m128d partialSumA = _mm256_extractf128_pd(sum, 0);
	__m128d partialSumB = _mm256_extractf128_pd(sum, 1);
	partialSumA = _mm_add_pd(partialSumA, partialSumB);
	partialSumA = _mm_hadd_pd(partialSumA, partialSumA);

	// convert to scalar result
	double low = _mm_cvtsd_f64(partialSumA);

	return stepScalar * low;
}
} // namespace PiLib
