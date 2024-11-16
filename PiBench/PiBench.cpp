#include <catch2/catch_all.hpp>
#include <numbers>

#include "../PiLib/PiLib.h"

TEST_CASE("Pi calculation is within +/- 0.01")
{
	constexpr int64_t ITERATION_SIZE = 10;
	constexpr double WITHIN_DELTA = 0.01;

	auto serialResult = PiLib::SerialPi(ITERATION_SIZE);
	REQUIRE_THAT(serialResult, Catch::Matchers::WithinAbs(std::numbers::pi, WITHIN_DELTA));
}

TEST_CASE("SerialPi calculation benchmark", "[!benchmark][SerialPi]")
{
	constexpr int64_t ITERATION_SIZE = 4026531839;

	BENCHMARK("SerialPi")
	{
		return PiLib::SerialPi(ITERATION_SIZE);
	};
}
