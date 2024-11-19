# CMakeList.txt : CMake project for PiLib

find_package(OpenMP)

add_library (PiLib "SerialPi.cpp" "SSE2Pi.cpp" "AvxPi.cpp" "Avx512Pi.cpp" "NaiveOmpPi.cpp" "FalseSharingOmpPi.cpp" "AtomicOmpPi.cpp" "ForOmpPi.cpp" "ThreadPi.cpp" "SimdThreadPi.cpp" "PiLib.h")
target_link_libraries(PiLib PRIVATE OpenMP::OpenMP_CXX)
target_compile_options(PiLib PRIVATE ${WARNING_OPTIONS})
target_compile_options(PiLib PRIVATE ${SANITIZER_OPTIONS})
target_link_options(PiLib PRIVATE ${SANITIZER_OPTIONS})
