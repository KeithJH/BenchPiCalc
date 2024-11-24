# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.25)

if (NOT DEFINED CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 20)
endif()

project ("BenchPiCalc")

#include(CheckIPOSupported)
#check_ipo_supported()
#set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
include(FetchContent)

if (MSVC)
  include("cmake/Warnings.msvc.cmake")

  #add_compile_options(/arch:AVX512)
  add_compile_options(/fp:fast)

  if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    include("cmake/Sanitizers.msvc.cmake")
  endif ()
else ()
  string(REPLACE "-O2" "-O3" CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")

  #add_compile_options(-march=native)
  add_compile_options(-ffast-math)

  include("cmake/Warnings.gcc.cmake")
  if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    include("cmake/Sanitizers.gcc.cmake")
  endif ()
endif ()

add_subdirectory ("PiLib")
add_subdirectory ("PiBench")
enable_testing ()
