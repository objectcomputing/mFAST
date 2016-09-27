

set(CMAKE_CXX_STANDARD 11 CACHE INTERNAL "specifies the C++ standard whose features are requested to build this target")
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if (${CMAKE_VERSION} VERSION_LESS "3.1.0" AND NOT ${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
  set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -std=c++${CMAKE_CXX_STANDARD})
endif()