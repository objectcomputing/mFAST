# The name of our project is "mFAST". CMakeLists files in this project can
# refer to the root source directory of the project as ${MFAST_SOURCE_DIR} and
# to the root binary directory of the project as ${MFAST_BINARY_DIR}.

cmake_minimum_required(VERSION 2.8.11)
project (mFAST CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

set(MFAST_MAJOR_VERSION 1)
set(MFAST_MINOR_VERSION 3)
set(MFAST_PATCH_VERSION 0)
set(MFAST_VERSION ${MFAST_MAJOR_VERSION}.${MFAST_MINOR_VERSION}.${MFAST_PATCH_VERSION})
set(MFAST_SOVERSION ${MFAST_MAJOR_VERSION}.${MFAST_MINOR_VERSION})

set(CPACK_PACKAGE_VERSION ${MFAST_VERSION})
set(CPACK_PACKAGE_NAME "mfast")

include(CPack)
include(SetInstallPaths)
include(Emscripten)
include(SetCXXStandard)
include(SetCompilerWarnings)
include(SetupCoverage)

# flag to enable building shared/dynamic library
set(BUILD_SHARED_LIBS OFF CACHE BOOL "build shared/dynamic library")

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

enable_testing()
add_subdirectory (tests)

if (BUILD_SHARED_LIBS)
  set(MFAST_LIBRARIES "${MFAST_SHARED_LIBRARIES}")
else (BUILD_SHARED_LIBS)
  set(MFAST_LIBRARIES "${MFAST_STATIC_LIBRARIES}")
endif (BUILD_SHARED_LIBS)

add_subdirectory (examples)


# Setting up dist target
# ===============================

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


