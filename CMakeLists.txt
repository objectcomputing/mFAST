# The name of our project is "mFAST". CMakeLists files in this project can
# refer to the root source directory of the project as ${MFAST_SOURCE_DIR} and
# to the root binary directory of the project as ${MFAST_BINARY_DIR}.

cmake_minimum_required(VERSION 2.8.0)


project (mFAST CXX)


set(EXT_PROJECTS_DIR external_projects)


if (CMAKE_BUILD_TYPE STREQUAL "Coverage")
	list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
	include(CodeCoverage)
	set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS_COVERAGE})
endif()

set(MFAST_MAJOR_VERSION 1)
set(MFAST_MINOR_VERSION 3)
set(MFAST_PATCH_VERSION 0)
set(MFAST_VERSION ${MFAST_MAJOR_VERSION}.${MFAST_MINOR_VERSION}.${MFAST_PATCH_VERSION})

set(CPACK_PACKAGE_VERSION ${MFAST_VERSION})
set(CPACK_PACKAGE_NAME "mfast")
include(CPack)



# flag to enable building shared/dynamic library
set(BUILD_SHARED_LIBS OFF CACHE BOOL "build shared/dynamic library")
# Offer the user the choice of overriding the installation directories
set(INSTALL_LIB_DIR lib CACHE PATH "Installation directory for libraries")
set(INSTALL_BIN_DIR bin CACHE PATH "Installation directory for executables")
set(INSTALL_INCLUDE_DIR include CACHE PATH
"Installation directory for header files")
set(INSTALL_DATA_DIR share CACHE PATH
"Installation directory for data files")

if (${CMAKE_SYSTEM_NAME} STREQUAL "Emscripten")
    if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
        set(CMAKE_INSTALL_PREFIX $ENV{EMSCRIPTEN}/system/local CACHE PATH "default install path" FORCE)
    endif()
    set(BUILD_FAST_TYPE_GEN OFF CACHE BOOL "Build fast_type_gen from source")
else()
    set(BUILD_FAST_TYPE_GEN ON CACHE BOOL "Build fast_type_gen from source")
endif()

# determine default path for CMake files
if(WIN32 AND NOT CYGWIN)
  set(DEF_INSTALL_CMAKE_DIR CMake)
else()
  if(${CMAKE_LIBRARY_ARCHITECTURE}) #support for multiarch debian-based distros, need cmake>=2.8.5
    set(DEF_INSTALL_CMAKE_DIR     lib/${CMAKE_LIBRARY_ARCHITECTURE}/cmake/mFAST)
  else()
    set(DEF_INSTALL_CMAKE_DIR     lib/cmake/mFAST)
  endif()
endif()
set(INSTALL_CMAKE_DIR ${DEF_INSTALL_CMAKE_DIR} CACHE PATH
"Installation directory for CMake files")

# Make relative paths absolute (needed later on)
foreach(p LIB BIN DATA CMAKE)
set(var INSTALL_${p}_DIR)
if(NOT IS_ABSOLUTE "${${var}}")
 set(${var} "${CMAKE_INSTALL_PREFIX}/${${var}}")
endif()
endforeach()

if(NOT IS_ABSOLUTE "${INSTALL_INCLUDE_DIR}")
 set(INSTALL_INCLUDE_DIR "${CMAKE_INSTALL_PREFIX}/include")
endif()


include_directories ("${PROJECT_BINARY_DIR}"
                     "${PROJECT_SOURCE_DIR}/src")

################################################################################
## Setting up Boost Library
################################


find_package(Boost 1.56.0 REQUIRED)


if (NOT BUILD_FAST_TYPE_GEN)
    find_program(FAST_TYPE_GEN NAMES fast_type_gen)
    if (${FAST_TYPE_GEN} STREQUAL "FAST_TYPE_GEN-NOTFOUND")
         message( FATAL_ERROR "Unable to find the required program fast_type_gen." )
    endif()
endif(NOT BUILD_FAST_TYPE_GEN)

include_directories(${Boost_INCLUDE_DIR})


################################################################################


# Select flags.
# Initialize CXXFLAGS.
if (CMAKE_COMPILER_IS_GNUCXX OR (${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang"))
    set(CMAKE_CXX_FLAGS "-W -Wall ${CMAKE_CXX_FLAGS} -std=c++11")
endif()
if (MSVC)
    add_definitions(/D_SCL_SECURE_NO_WARNINGS /D_CRT_SECURE_NO_WARNINGS)
endif()

add_definitions(-DSIZEOF_VOID_P=${CMAKE_SIZEOF_VOID_P})

if (${CMAKE_SYSTEM_NAME} STREQUAL "Emscripten")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-warn-absolute-paths -s AGGRESSIVE_VARIABLE_ELIMINATION=1 -s OUTLINING_LIMIT=20000")
endif()
# variables to be used later
set(MFAST_DYNAMIC_COMPONENTS CACHE INTERNAL "")
set(MFAST_STATIC_COMPONENTS CACHE INTERNAL "")
set(MFAST_LIBRARY CACHE INTERNAL "")

############################################################################
# Setting up RPATH
#

# add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)


############################################################################


add_subdirectory (src)

link_directories (${PROJECT_BINARY_DIR}/src)

#============================================================
# Install fast_type_gen batch file to set up the PATH environment
# variable for using BOOST libraries

# if(MSVC_IDE)
#     configure_file(invoke.bat.in
#                    "${PROJECT_BINARY_DIR}/bin/invoke.bat"
#                    @ONLY)
# endif(MSVC_IDE)

#============================================================
# FASTTYPEGEN_TARGET (public function)
#============================================================
#

function (FASTTYPEGEN_TARGET Name)
  set(FASTTYPEGEN_TARGET_usage "FASTTYPEGEN_TARGET(<Name> Input1 Input2 ...]" PARENT_SCOPE)

  set("FASTTYPEGEN_${Name}_OUTPUTS")
  set(INPUTS)

  foreach (input IN LISTS ARGN)
    get_filename_component(noext_name "${input}" NAME_WE)
    list(APPEND "FASTTYPEGEN_${Name}_OUTPUTS"
         "${CMAKE_CURRENT_BINARY_DIR}/${noext_name}.cpp"
         "${CMAKE_CURRENT_BINARY_DIR}/${noext_name}.h"
         "${CMAKE_CURRENT_BINARY_DIR}/${noext_name}.inl")
    list(APPEND INPUTS "${CMAKE_CURRENT_SOURCE_DIR}/${input}")
  endforeach (input)

  if (BUILD_FAST_TYPE_GEN)
    set(FAST_TYPE_GEN fast_type_gen)
    set(FAST_TYPE_GEN_TARGET fast_type_gen)
  else (BUILD_FAST_TYPE_GEN)
    set(FAST_TYPE_GEN_TARGET)
  endif (BUILD_FAST_TYPE_GEN)

  add_custom_command(
      OUTPUT ${FASTTYPEGEN_${Name}_OUTPUTS}
      COMMAND "${FAST_TYPE_GEN}" ${INPUTS}
      DEPENDS ${ARGN} ${FAST_TYPE_GEN_TARGET}
      COMMENT "[FASTTYPEGEN][${Name}] Building Fast Application Types"
      WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
      VERBATIM)

  include_directories(BEFORE "${CMAKE_CURRENT_BINARY_DIR}")

  set("FASTTYPEGEN_${Name}_OUTPUTS" "${FASTTYPEGEN_${Name}_OUTPUTS}" PARENT_SCOPE)
  set("FASTTYPEGEN_${Name}_DEFINED" TRUE PARENT_SCOPE)
  set("FASTTYPEGEN_${Name}_INPUTS" "${ARGN}" PARENT_SCOPE)
endfunction ()



# Includes Catch in the project:
#add_subdirectory(${EXT_PROJECTS_DIR}/catch)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/catch/include ${COMMON_INCLUDES})
enable_testing()

add_subdirectory (tests)

if (BUILD_SHARED_LIBS)
  set(MFAST_LIBRARIES mfast_coder mfast_xml_parser mfast)
  add_definitions( -DMFAST_DYN_LINK )
else()
  set(MFAST_LIBRARIES mfast_coder_static mfast_xml_parser_static mfast_static)
endif()

add_subdirectory (examples)


# Setting up dist target
# ===============================

set(ARCHIVE_NAME ${CMAKE_PROJECT_NAME}-${MFAST_VERSION})
add_custom_target(dist
    COMMAND git archive --prefix=${ARCHIVE_NAME}/ HEAD
        | bzip2 > ${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}.tar.bz2
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

# Provide mfast-config.cmake and mfast-config.version to be used by other applications
# ===============================

if (NOT ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} LESS 2.8)
  export(PACKAGE ${CMAKE_PROJECT_NAME})
endif(NOT ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} LESS 2.8)

if (BUILD_FAST_TYPE_GEN)
  export(TARGETS fast_type_gen ${MFAST_LIBRARIES} FILE "${PROJECT_BINARY_DIR}/mFASTTargets.cmake")
else()
  set(FAST_TYPE_GEN_INSTALL_LOCATION ${FAST_TYPE_GEN})
endif()

# Create the mFASTConfig.cmake for the build tree
set(CONF_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/src" "${PROJECT_BINARY_DIR}")
configure_file(mFASTConfig.cmake.in
               "${PROJECT_BINARY_DIR}/mFASTConfig.cmake"
               @ONLY)

# Create the mFASTConfig.cmake for the install tree
file(RELATIVE_PATH REL_INCLUDE_DIR "${INSTALL_CMAKE_DIR}" "${INSTALL_INCLUDE_DIR}")
set(CONF_INCLUDE_DIRS "\${MFAST_CMAKE_DIR}/${REL_INCLUDE_DIR}")
configure_file(mFASTConfig.cmake.in
               "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/mFASTConfig.cmake"
               @ONLY)

# Create the mFASTConfigVersion.cmake for both
configure_file(mFASTConfigVersion.cmake.in
               "${PROJECT_BINARY_DIR}/mFASTConfigVersion.cmake"
               @ONLY)

# Install the mFASTConfig.cmake and mFASTConfigVersion.cmake
install(FILES
  "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/mFASTConfig.cmake"
  "${PROJECT_BINARY_DIR}/mFASTConfigVersion.cmake"
  DESTINATION "${INSTALL_CMAKE_DIR}" COMPONENT dev)

# Install the export set for use with the install-tree
install(EXPORT mFASTTargets DESTINATION "${INSTALL_CMAKE_DIR}" COMPONENT dev)

file(GLOB schema_files "${CMAKE_CURRENT_SOURCE_DIR}/schema/*.*")

install(FILES
        ${schema_files}
        DESTINATION ${INSTALL_DATA_DIR}/mfast)


if (CMAKE_BUILD_TYPE STREQUAL "Coverage")
    SETUP_TARGET_FOR_COVERAGE(
                    coverage            # Name for custom target.
                    mfast_test          # Name of the test driver executable that runs the tests.
                                        # NOTE! This should always have a ZERO as exit code
                                        # otherwise the coverage generation will not complete.
                    coverage            # Name of output directory.
                    )
endif()
