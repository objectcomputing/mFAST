# The name of our project is "TinyFAST". CMakeLists files in this project can
# refer to the root source directory of the project as ${MFAST_SOURCE_DIR} and
# to the root binary directory of the project as ${MFAST_BINARY_DIR}.
cmake_minimum_required (VERSION 2.8)
project (TinyFAST)


set(MFAST_MAJOR_VERSION 0)
set(MFAST_MINOR_VERSION 1)
set(MFAST_PATCH_VERSION 0)
set(MFAST_VERSION ${MFAST_MAJOR_VERSION}.${MFAST_MINOR_VERSION}.${MFAST_PATCH_VERSION})



# Offer the user the choice of overriding the installation directories
set(INSTALL_LIB_DIR lib CACHE PATH "Installation directory for libraries")
set(INSTALL_BIN_DIR bin CACHE PATH "Installation directory for executables")
set(INSTALL_INCLUDE_DIR include CACHE PATH
"Installation directory for header files")
set(INSTALL_DATA_DIR share CACHE PATH
"Installation directory for data files")

# Make relative paths absolute (needed later on)
foreach(p LIB BIN DATA)
set(var INSTALL_${p}_DIR)
if(NOT IS_ABSOLUTE "${${var}}")
 set(${var} "${CMAKE_INSTALL_PREFIX}/${${var}}")
endif()
endforeach()

if(NOT IS_ABSOLUTE "${INSTALL_INCLUDE_DIR}")
 set(INSTALL_INCLUDE_DIR "${CMAKE_INSTALL_PREFIX}/include")
endif()

##########################################################################################################
## Setting up Boost Library
################################

find_package( Boost 1.53.0 REQUIRED unit_test_framework system filesystem)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})

###########################################################################################################


# Select flags.
# Initialize CXXFLAGS.
set(CMAKE_CXX_FLAGS_DEBUG          "-O0 -g")
set(CMAKE_CXX_FLAGS_RELEASE        "-O3 -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")


include_directories ("${PROJECT_BINARY_DIR}"
                     "${PROJECT_SOURCE_DIR}/src")
add_subdirectory (src)

link_directories (${PROJECT_BINARY_DIR}/src)
set(MFAST_LIBRARIES mfast)

#============================================================
# FASTTYPEGEN_TARGET (public macro)
#============================================================
#
set(FASTTYPEGEN_EXECUTABLE ${CMAKE_BINARY_DIR}/src/fast_type_gen/fast_type_gen)

macro(FASTTYPEGEN_TARGET Name)
set(FASTTYPEGEN_TARGET_usage "FASTTYPEGEN_TARGET(<Name> Input1 Input2 ...]")

foreach (input ${ARGN})
	get_filename_component(noext_name ${input} NAME_WE)
	set(FASTTYPEGEN_${Name}_INPUTS_NOEXT ${FASTTYPEGEN_${Name}_INPUTS_NOEXT} ${noext_name})
endforeach(input)

foreach(var ${FASTTYPEGEN_${Name}_INPUTS_NOEXT})
	set(FASTTYPEGEN_${Name}_OUTPUTS ${FASTTYPEGEN_${Name}_OUTPUTS} ${CMAKE_CURRENT_SOURCE_DIR}/${var}.cpp)
endforeach(var)

add_custom_command(OUTPUT ${FASTTYPEGEN_${Name}_OUTPUTS}
  COMMAND ${FASTTYPEGEN_EXECUTABLE}
  ARGS ${ARGN}
  DEPENDS ${ARGN} ${FASTTYPEGEN_EXECUTABLE}
  COMMENT "[FASTTYPEGEN][${Name}] Building Fast Application Types"
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

set(FASTTYPEGEN_${Name}_DEFINED TRUE)
set(FASTTYPEGEN_${Name}_INPUTS ${ARGN})
endmacro()
#============================================================

# Recurse into the "src" and "unit-tests" subdirectories. This does not actually
# cause another cmake executable to run. The same process will walk through
# the project's entire directory structure.

add_subdirectory (tests)
add_subdirectory (examples)

# Setting up dist target
# ===============================

set(ARCHIVE_NAME ${CMAKE_PROJECT_NAME}-${MFAST_VERSION})
add_custom_target(dist
    COMMAND git archive --prefix=${ARCHIVE_NAME}/ HEAD
        | bzip2 > ${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}.tar.bz2
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
