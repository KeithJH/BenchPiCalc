# SSE2Pi

Serial solution using SSE2 vector instructions. When compiling x86\_64 these instructions are enabled by default, meaning the compiler was free to optimize even the SerialPi solution with the same instruction set. Manually using intrinsics allows for finer control and results in better performance. This roughly halves the performance of the Default SerialPi solution, which is on par with what one would expect going from scalar to vector instructions of two packed doubles.
