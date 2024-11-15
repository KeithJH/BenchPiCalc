# CMakeList.txt : CMake project for PiBench

add_executable (PiBench "PiBench.cpp")
target_link_libraries(PiBench PRIVATE PiLib)
target_link_libraries(PiBench PRIVATE Catch2::Catch2WithMain)
target_compile_options(PiBench PRIVATE ${WARNING_OPTIONS})
target_compile_options(PiBench PRIVATE ${SANITIZER_OPTIONS})
target_link_options(PiBench PRIVATE ${SANITIZER_OPTIONS})

# Register Tests
include(CTest)
include(Catch)
catch_discover_tests(PiBench)
