#include <catch2/catch_all.hpp>
#include <numbers>

#include "../PiLib/PiLib.h"

namespace PiBench
{
constexpr int64_t TEST_ITERATION_SIZE = 10;
constexpr int64_t SMALL_BENCH_ITERATION_SIZE = 4026531839;

TEST_CASE("Pi calculation is within +/- 0.01", "[pi]")
{
	constexpr double WITHIN_DELTA = 0.01;

	REQUIRE_THAT(PiLib::SerialPi(TEST_ITERATION_SIZE), Catch::Matchers::WithinAbs(std::numbers::pi, WITHIN_DELTA));
	REQUIRE_THAT(PiLib::SSE2Pi(TEST_ITERATION_SIZE), Catch::Matchers::WithinAbs(std::numbers::pi, WITHIN_DELTA));

	if (PiLib::IsAvxPiSupported())
	{
		REQUIRE_THAT(PiLib::AvxPi(TEST_ITERATION_SIZE), Catch::Matchers::WithinAbs(std::numbers::pi, WITHIN_DELTA));
	}
	else
	{
		WARN("PiLib::AvxPi not tested as it is not supported");
	}
}

TEST_CASE("SerialPi calculation benchmark", "[!benchmark][SerialPi][pi]")
{
	BENCHMARK("SerialPi")
	{
		return PiLib::SerialPi(SMALL_BENCH_ITERATION_SIZE);
	};
}

TEST_CASE("SSE2Pi calculation benchmark", "[!benchmark][SSE2Pi][pi]")
{
	BENCHMARK("SSE2Pi")
	{
		return PiLib::SSE2Pi(SMALL_BENCH_ITERATION_SIZE);
	};
}

TEST_CASE("AvxPi calculation benchmark", "[!benchmark][AvxPi][pi]")
{
	if (PiLib::IsAvxPiSupported())
	{
		BENCHMARK("AvxPi")
		{
			return PiLib::AvxPi(SMALL_BENCH_ITERATION_SIZE);
		};
	}
	else
	{
		WARN("PiLib::AvxPi not tested as it is not supported");
	}
}
}
