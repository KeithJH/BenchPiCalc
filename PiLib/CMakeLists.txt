# CMakeList.txt : CMake project for PiLib

add_library (PiLib "SerialPi.cpp" "SSE2Pi.cpp" "PiLib.h")
target_compile_options(PiLib PRIVATE ${WARNING_OPTIONS})
target_compile_options(PiLib PRIVATE ${SANITIZER_OPTIONS})
target_link_options(PiLib PRIVATE ${SANITIZER_OPTIONS})
