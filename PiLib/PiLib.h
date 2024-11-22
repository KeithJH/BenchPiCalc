#pragma once
#include <cstdint>
#include <cstddef>

namespace PiLib
{
double SerialPi(const std::size_t iterations);
double SSE2Pi(const std::size_t iterations);

bool IsAvxPiSupported();
double AvxPi(const std::size_t iterations);

bool IsAvx512PiSupported();
double Avx512Pi(const std::size_t iterations);

double NaiveOmpPi(const std::size_t iterations);
double NaiveOmpPi(const std::size_t iterations, const std::size_t threadCount);

double FalseSharingOmpPi(const std::size_t iterations);
double FalseSharingOmpPi(const std::size_t iterations, const std::size_t threadCount);

double AtomicOmpPi(const std::size_t iterations);
double AtomicOmpPi(const std::size_t iterations, const std::size_t threadCount);

double ForOmpPi(const std::size_t iterations);
double ForOmpPi(const std::size_t iterations, const std::size_t threadCount);

double ThreadPi(const std::size_t iterations);
double ThreadPi(const std::size_t iterations, std::size_t threadCount);

bool IsSimdThreadPiSupported();
double SimdThreadPi(const std::size_t iterations);
double SimdThreadPi(const std::size_t iterations, std::size_t threadCount);

double KomputePi(const std::size_t iterations);
double KomputePi(const std::size_t iterations, const uint32_t deviceIndex);
} // namespace PiLib
