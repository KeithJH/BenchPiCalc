#include <catch2/catch_all.hpp>
#include <numbers>
#include <string>

#include "../PiLib/PiLib.h"

namespace PiBench
{
constexpr int64_t TEST_ITERATION_SIZE = 10;
constexpr int64_t SMALL_BENCH_ITERATION_SIZE = 4026531839;
constexpr double WITHIN_DELTA = 0.01;

template <typename EvaluateFunc>
void TestPiFunction(bool isSupported, EvaluateFunc evaluateFunc, int64_t iterationCount)
{
	if (isSupported)
	{
		// TODO: Using a function argument like this hides which function was called
		CHECK_THAT(evaluateFunc(iterationCount), Catch::Matchers::WithinAbs(std::numbers::pi, WITHIN_DELTA));
	}
	else
	{
		WARN("Unsupported");
	}
}

// Verify results are in the correct ballpark
TEST_CASE("SerialPi calculation is within +/- 0.01", "[pi][SerialPi]")
{
	TestPiFunction(true, PiLib::SerialPi, TEST_ITERATION_SIZE);
}

TEST_CASE("SSE2Pi calculation is within +/- 0.01", "[pi][SSE2Pi]")
{
	TestPiFunction(true, PiLib::SSE2Pi, TEST_ITERATION_SIZE);
}

TEST_CASE("AvxPi calculation is within +/- 0.01", "[pi][AvxPi]")
{
	TestPiFunction(PiLib::IsAvxPiSupported(), PiLib::AvxPi, TEST_ITERATION_SIZE);
}

TEST_CASE("Avx512Pi calculation is within +/- 0.01", "[pi][Avx512Pi]")
{
	TestPiFunction(PiLib::IsAvx512PiSupported(), PiLib::Avx512Pi, TEST_ITERATION_SIZE);
}

TEST_CASE("NaiveOmpPi calculation is within +/- 0.01", "[pi][NaiveOmpPi]")
{
	CHECK_THAT(PiLib::NaiveOmpPi(TEST_ITERATION_SIZE), Catch::Matchers::WithinAbs(std::numbers::pi, WITHIN_DELTA));
}

TEST_CASE("FalseSharingOmpPi calculation is within +/- 0.01", "[pi][FalseSharingOmpPi]")
{
	CHECK_THAT(PiLib::FalseSharingOmpPi(TEST_ITERATION_SIZE), Catch::Matchers::WithinAbs(std::numbers::pi, WITHIN_DELTA));
}

// Benchmarks
template <typename EvaluateFunc>
void BenchmarkPiFunction(std::string &&benchmarkName, bool isSupported, EvaluateFunc evaluateFunc,
                         int64_t iterationCount)
{
	if (isSupported)
	{
		BENCHMARK(std::move(benchmarkName))
		{
			return evaluateFunc(iterationCount);
		};
	}
	else
	{
		WARN("Unsupported");
	}
}

TEST_CASE("SerialPi calculation benchmark", "[!benchmark][SerialPi][pi]")
{
	BenchmarkPiFunction("SerialPi", true, PiLib::SerialPi, SMALL_BENCH_ITERATION_SIZE);
}

TEST_CASE("SSE2Pi calculation benchmark", "[!benchmark][SSE2Pi][pi]")
{
	BenchmarkPiFunction("SSE2Pi", true, PiLib::SSE2Pi, SMALL_BENCH_ITERATION_SIZE);
}

TEST_CASE("AvxPi calculation benchmark", "[!benchmark][AvxPi][pi]")
{
	BenchmarkPiFunction("AvxPi", PiLib::IsAvxPiSupported(), PiLib::AvxPi, SMALL_BENCH_ITERATION_SIZE);
}

TEST_CASE("Avx512Pi calculation benchmark", "[!benchmark][Avx512Pi][pi]")
{
	BenchmarkPiFunction("Avx512Pi", PiLib::IsAvx512PiSupported(), PiLib::Avx512Pi, SMALL_BENCH_ITERATION_SIZE);
}

TEST_CASE("NaiveOmpPi calculation benchmark", "[!benchmark][NaiveOmpPi][pi]")
{
	BENCHMARK("NaiveOmpPi")
	{
		return PiLib::NaiveOmpPi(SMALL_BENCH_ITERATION_SIZE);
	};
}

TEST_CASE("FalseSharingOmpPi calculation benchmark", "[!benchmark][FalseSharingOmpPi][pi]")
{
	BENCHMARK("FalseSharingOmpPi")
	{
		return PiLib::FalseSharingOmpPi(SMALL_BENCH_ITERATION_SIZE);
	};
}
} // namespace PiBench
