

set(CMAKE_CXX_STANDARD 11 CACHE INTERNAL "specifies the C++ standard whose features are requested to build this target")
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if (NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  ### Even though CMAKE_CXX_STANDARD_REQUIRED is supported since CMake 3.1, it doesn't work for Emscripten em++ together with
  ### Cmake 3.6.
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++${CMAKE_CXX_STANDARD}")
endif()