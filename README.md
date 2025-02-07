# BenchPiCalc
## Goal
Benchmark various implementations of calculating pi, focusing on threading, vectorization, and OpenMP. The solutions are inspired and extended from [Introduction to OpenMP - Tim Mattson (Intel)](https://www.youtube.com/playlist?list=PLLX-Q6B8xqZ8n8bwjGdzBJ25X2utwnoEG), which introduces the problem as a great candidate for parallelization that exhibits easily observable performance benefits. The series focuses on solutions that leverage OpenMP, but the problem is also easily extendable to leverage other techniques as well. The focus is not to calculate pi as quickly as possible (you'd be better off just using `std::numbers::pi`, after all) but to explore performance implications of various configurations and solutions.

## General Solution
All implementations solve the integration of 4/(1 + x^2) from 0 to 1 using a Riemann sum with various numbers of partitions.

## Implementation Notes
* [SerialPi](./notes/SerialPi.md): Basic serial solution without anything fancy
* [SSE2Pi](./notes/SSE2Pi.md): Serial solution using SSE2 vector instructions
* [AvxPi](./notes/AvxPi.md): Serial solution using AVX vector instructions
* [Avx512Pi](./notes/Avx512Pi.md): Serial solution using AVX512 vector instructions
* [NaiveOmpPi](./notes/NaiveOmpPi.md): Parallel solution using basic OpenMP constructs
* [FalseSharingOmpPi](./notes/FalseSharingOmpPi.md): Parallel solution using basic OpenMP constructs, but causing a false sharing issue
* [AtomicOmpPi](./notes/AtomicOmpPi.md): Parallel solution using basic OpenMP constructs, including an atomic for sum increment
* [ForOmpPi](./notes/ForOmpPi.md): Parallel solution using an OpenMP parallel for loop with a reduction clause for sum
* [ThreadPi](./notes/ThreadPi.md): Parallel solution manually using `std::thread`
* [SimdThreadPi](./notes/SimdThreadPi.md): Parallel solution manually using `std::thread` and AVX512 vector instructions
* [KomputePi](./notes/KomputePi.md): General Purpose GPU (GPGPU) solution using Vulkan compute shaders dispatched using `kompute`

## Results Summary
4026531839 Iterations on an AMD Ryzen 7 7700X (CPU tests) or a Radeon RX 6900 XT (GPU tests)

| Solution          | threads | -march=native | -ffast-math | time (ms) |
|-------------------|---------|---------------|-------------|-----------|
| SerialPi          | 1       | yes           | yes         |   831.513 |
| SerialPi          | 1       | yes           | no          |   2213.48 |
| SerialPi          | 1       | no            | *           |   3434.23 |
| SSE2Pi            | 1       | *             | *           |   1641.54 |
| AvxPi             | 1       | *             | *           |   820.518 |
| Avx512Pi          | 1       | *             | *           |   827.005 |
| NaiveOmpPi        | 16      | no            | *           |   439.549 |
| FalseSharingOmpPi | 16      | no            | *           |   6803.25 |
| AtomicOmpPi       | 16      | no            | *           |   446.516 |
| ForOmpPi          | 16      | no            | *           |   447.340 |
| ForOmpPi          | 16      | yes           | no          |   154.168 |
| ForOmpPi          | 16      | yes           | yes         |   103.906 |
| ThreadPi          | 16      | yes           | *           |   413.089 |
| SimdThreadPi      | 16      | *             | *           |   104.933 |
| KomputePi         | GPU     | *             | *           |   159.680 |

## Building
The project is setup with `CMake` using `FetchContent` for a few packages.
* [`Catch2`](https://github.com/catchorg/Catch2) is used for testing and benchmarking.
* [`kompute`](https://github.com/KomputeProject/kompute) is used for running GPU workloads with Vulkan compute shaders.

It also requires manually downloading and installing:
  * The [Vulkan SDK](https://vulkan.lunarg.com/) for `kompute` support

Presets are created for `g++` as that is the compiler everything is currently test with. The "linux-gcc-profile" preset is meant to be just as optimized as the "release" preset but with debug information, however, on some machines (Intel CPUs?) this causes some performance issues.
```
$ cmake --list-presets
Available configure presets:

  "linux-gcc-debug"   - Linux GCC Debug
  "linux-gcc-release" - Linux GCC Release
  "linux-gcc-profile" - Linux GCC Release with debug info
```

The project can be configured with:
```
$ cmake --preset linux-gcc-profile
```

The project can then be built with:
```
$ cmake --build out/build/linux-gcc-profile/
```

And then tested/run with:
```
$ ctest --test-dir out/build/linux-gcc-profile/
$ ./out/build/linux-gcc-profile/PiBench/PiBench [common Catch2 args to control what to run]
```
In particular the following arguments are a good starting point for running benchmarks, as they are skipped by default:
```
$ ./out/build/linux-gcc-profile/PiBench/PiBench --benchmark-no-analysis --benchmark-samples 1 "[!benchmark]"
Filters: [!benchmark]
Randomness seeded to: 2212001003

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PiBench is a Catch2 v3.6.0 host application.
Run with -? for options

-------------------------------------------------------------------------------
Pi calculation benchmark
-------------------------------------------------------------------------------
/home/keifer/source/repos/BenchPiCalc/PiBench/PiBench.cpp:15
...............................................................................

benchmark name                            samples    iterations          mean
-------------------------------------------------------------------------------
SerialPi                                         1             1     3.43423 s
```
