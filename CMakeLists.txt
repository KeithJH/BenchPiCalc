# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.9)

if (NOT DEFINED CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 20)
endif()

project ("BenchPiCalc")

#include(CheckIPOSupported)
#check_ipo_supported()
#set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

find_package(Catch2 3 REQUIRED)

if (MSVC)
  set(WARNING_OPTIONS /W4 /permissive- )
  list(APPEND WARNING_OPTIONS /WX)

  if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(SANITIZER_OPTIONS -fsanitize=address)
  endif ()
else ()
  #add_compile_options(-march=native)
  add_compile_options(-ffast-math)

  set(WARNING_OPTIONS -Wall -Wextra -Wshadow -Wold-style-cast -Wunused -Wpedantic -Wcast-align -Wimplicit-fallthrough)
  list(APPEND WARNING_OPTIONS -Werror)

  if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(SANITIZER_OPTIONS "-fsanitize=address,leak,undefined")
    #set(SANITIZER_OPTIONS "-fsanitize=memory")
    #set(SANITIZER_OPTIONS "-fsanitize=thread")
  endif ()
endif ()

add_subdirectory ("PiLib")
add_subdirectory ("PiBench")
enable_testing ()
