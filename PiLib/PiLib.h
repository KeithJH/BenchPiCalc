#pragma once
#include <cstdint>


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

double AtomicOmpPi(const int64_t iterations);
double AtomicOmpPi(const int64_t iterations, std::size_t threadCount);

double ForOmpPi(const int64_t iterations);
double ForOmpPi(const int64_t iterations, std::size_t threadCount);

double ThreadPi(const int64_t iterations);
double ThreadPi(const int64_t iterations, std::size_t threadCount);

bool IsSimdThreadPiSupported();
double SimdThreadPi(const int64_t iterations);
double SimdThreadPi(const int64_t iterations, std::size_t threadCount);

double KomputePi(const int64_t iterations);
double KomputePi(const int64_t iterations, const int32_t deviceIndex);
} // namespace PiLib
