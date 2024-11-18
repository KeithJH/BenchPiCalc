#pragma once
#include <cstdint>
#include <numbers>

namespace PiLib
{
double SerialPi(const int64_t iterations);
double SSE2Pi(const int64_t iterations);

bool IsAvxPiSupported();
double AvxPi(const int64_t iterations);

bool IsAvx512PiSupported();
double Avx512Pi(const int64_t iterations);

double NaiveOmpPi(const int64_t iterations);
double NaiveOmpPi(const int64_t iterations, std::size_t threadCount);

double FalseSharingOmpPi(const int64_t iterations);
double FalseSharingOmpPi(const int64_t iterations, std::size_t threadCount);
} // namespace PiLib
