# The name of our project is "mFAST". CMakeLists files in this project can
# refer to the root source directory of the project as ${MFAST_SOURCE_DIR} and
# to the root binary directory of the project as ${MFAST_BINARY_DIR}.

cmake_minimum_required(VERSION 2.8.11)
project (mFAST CXX)

# options to build tests, examples and packages
option(BUILD_TESTS "Build tests" ON)
option(BUILD_EXAMPLES "Build examples" ON)
option(BUILD_PACKAGES "Build packages" ON)

# debug build by default
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build" FORCE)
endif(NOT CMAKE_BUILD_TYPE)

if(POLICY CMP0054)
    cmake_policy(SET CMP0054 OLD)
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(MFAST_MAJOR_VERSION 1)
set(MFAST_MINOR_VERSION 3)
set(MFAST_PATCH_VERSION 0)
set(MFAST_VERSION ${MFAST_MAJOR_VERSION}.${MFAST_MINOR_VERSION}.${MFAST_PATCH_VERSION})
set(MFAST_SOVERSION ${MFAST_MAJOR_VERSION}.${MFAST_MINOR_VERSION})

include(SetInstallPaths)
include(Emscripten)
include(SetCXXStandard)
include(SetCompilerWarnings)
include(SetupCoverage)

# flag to enable building shared/dynamic library
set(BUILD_SHARED_LIBS OFF CACHE BOOL "build shared/dynamic library")

if(BUILD_PACKAGES)
  # Build deb package by default
  if(NOT CPACK_GENERATOR)
    set(CPACK_GENERATOR "DEB" CACHE STRING "List of generators.")
  endif(NOT CPACK_GENERATOR)

  if(CUSTOM_INSTALL_PREFIX)
    set(CMAKE_INSTALL_PREFIX ${CUSTOM_INSTALL_PREFIX} CACHE STRING "Custom installation prefix.")
    set(CPACK_SET_DESTDIR true)
    set(CPACK_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
  endif(CUSTOM_INSTALL_PREFIX)

  set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A FAST (FIX Adapted for STreaming) encoder/decoder")
  set(CPACK_PACKAGE_NAME "mfast")
  set(CPACK_PACKAGE_VERSION ${MFAST_VERSION})
  set(CPACK_PACKAGE_VENDOR "Object Computing, Inc.")
  set(CPACK_PACKAGE_CONTACT "info@ociweb.com")
  set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Huang-Ming Huang")
  set(CPACK_DEBIAN_PACKAGE_SECTION "devel")

  include(CPack)

endif(BUILD_PACKAGES)

find_package(Boost 1.56.0 REQUIRED)
include_directories(${Boost_INCLUDE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/tinyxml2)
include_directories ("${PROJECT_SOURCE_DIR}/src")

add_definitions(-DSIZEOF_VOID_P=${CMAKE_SIZEOF_VOID_P})

# variables to be used later
set(MFAST_SHARED_LIBRARIES CACHE INTERNAL "")
set(MFAST_STATIC_LIBRARIES CACHE INTERNAL "")
set(MFAST_DYNAMIC_COMPONENTS CACHE INTERNAL "")
set(MFAST_STATIC_COMPONENTS CACHE INTERNAL "")

set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

add_subdirectory (src)
include(FastTypeGenTarget)

if(BUILD_TESTS)
  enable_testing()
  add_subdirectory (tests)
endif(BUILD_TESTS)

if (BUILD_SHARED_LIBS)
  set(MFAST_LIBRARIES "${MFAST_SHARED_LIBRARIES}")
else (BUILD_SHARED_LIBS)
  set(MFAST_LIBRARIES "${MFAST_STATIC_LIBRARIES}")
endif (BUILD_SHARED_LIBS)

if(BUILD_EXAMPLES)
  add_subdirectory (examples)
endif(BUILD_EXAMPLES)

# Setting up dist target
set(ARCHIVE_NAME ${CMAKE_PROJECT_NAME}-${MFAST_VERSION})
add_custom_target(dist
    COMMAND git archive --prefix=${ARCHIVE_NAME}/ HEAD
        | bzip2 > ${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}.tar.bz2
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

include(GenProjectConfig)

file(GLOB schema_files "${CMAKE_CURRENT_SOURCE_DIR}/schema/*.*")
install(FILES
        ${schema_files}
        DESTINATION ${INSTALL_DATA_DIR}/mfast)

# Print summary
message(STATUS "")
message(STATUS "CMAKE_COMMAND: " ${CMAKE_COMMAND})
message(STATUS "CMAKE_SYSTEM: " ${CMAKE_SYSTEM})
message(STATUS "CMAKE_SYSTEM_PROCESSOR: " ${CMAKE_SYSTEM_PROCESSOR})

message(STATUS "CMAKE_CXX_COMPILER: " ${CMAKE_CXX_COMPILER})
message(STATUS "CMAKE_CXX_FLAGS: " ${CMAKE_CXX_FLAGS})
message(STATUS "CMAKE_BUILD_TYPE: " ${CMAKE_BUILD_TYPE})

message(STATUS "BUILD_SHARED_LIBS: " ${BUILD_SHARED_LIBS})
message(STATUS "BUILD_TESTS: " ${BUILD_TESTS})
message(STATUS "BUILD_EXAMPLES: " ${BUILD_EXAMPLES})
message(STATUS "BUILD_PACKAGES: " ${BUILD_PACKAGES})
message(STATUS "")
