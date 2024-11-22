# CMakeList.txt : CMake project for PiLib
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(KOMPUTE_OPT_LOG_LEVEL Debug CACHE STRING "Kompute log level")
else ()
  set(KOMPUTE_OPT_LOG_LEVEL Error CACHE STRING "Kompute log level")
endif ()

FetchContent_Declare(kompute
  #GIT_REPOSITORY https://github.com/KomputeProject/kompute.git
  #GIT_TAG 73f0e28e1455807ce618557eedeba8b12370aa64

  # There are some changes that have yet to be pushed to the official repo,
  # so use a development fork for now
  GIT_REPOSITORY https://github.com/KeithJH/kompute.git
  GIT_TAG dev
  SYSTEM
)
FetchContent_MakeAvailable(kompute)

vulkan_compile_shader(
    INFILE shaders/PartialSum.comp
    OUTFILE shaders/PartialSum.hpp
    ARGS "--target-env" "vulkan1.1"
    NAMESPACE "shaders"
)

vulkan_compile_shader(
    INFILE shaders/Reduce.comp
    OUTFILE shaders/Reduce.hpp
    ARGS "--target-env" "vulkan1.1"
    NAMESPACE "shaders"
)

add_library(shaders INTERFACE "shaders/Reduce.hpp" "shaders/PartialSum.hpp")
target_include_directories(shaders INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

find_package(OpenMP)

add_library (PiLib
  "SerialPi.cpp" "SSE2Pi.cpp" "AvxPi.cpp" "Avx512Pi.cpp"
  "NaiveOmpPi.cpp" "FalseSharingOmpPi.cpp" "AtomicOmpPi.cpp" "ForOmpPi.cpp"
  "ThreadPi.cpp" "SimdThreadPi.cpp"
  "KomputePi.cpp"
  "PiLib.h")
target_compile_options(PiLib PRIVATE ${WARNING_OPTIONS})
target_compile_options(PiLib PRIVATE ${SANITIZER_OPTIONS})
target_link_options(PiLib PRIVATE ${SANITIZER_OPTIONS})
target_link_libraries(PiLib PRIVATE OpenMP::OpenMP_CXX kompute::kompute shaders)
