# The name of our project is "mFAST". CMakeLists files in this project can
# refer to the root source directory of the project as ${MFAST_SOURCE_DIR} and
# to the root binary directory of the project as ${MFAST_BINARY_DIR}.
cmake_minimum_required (VERSION 2.8)
project (mFAST)


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

find_package( Boost 1.54.0 REQUIRED unit_test_framework system filesystem)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})

###########################################################################################################


# Select flags.
# Initialize CXXFLAGS.
if (CMAKE_COMPILER_IS_GNUCXX OR ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang"))
	set(CMAKE_CXX_FLAGS "-W -Wall ${CMAKE_CXX_FLAGS}")
elseif (MSVC)
	add_definitions(/D_SCL_SECURE_NO_WARNINGS /D_CRT_SECURE_NO_WARNINGS)
endif()

include_directories ("${PROJECT_BINARY_DIR}"
                     "${PROJECT_SOURCE_DIR}/src")
add_subdirectory (src)

link_directories (${PROJECT_BINARY_DIR}/src)
set(MFAST_LIBRARIES mfast)

#============================================================
# FASTTYPEGEN_TARGET (public macro)
#============================================================
#

macro(FASTTYPEGEN_TARGET Name)
set(FASTTYPEGEN_TARGET_usage "FASTTYPEGEN_TARGET(<Name> Input1 Input2 ...]")

foreach (input ${ARGN})
	get_filename_component(noext_name ${input} NAME_WE)
	set(FASTTYPEGEN_${Name}_INPUTS_NOEXT ${FASTTYPEGEN_${Name}_INPUTS_NOEXT} ${noext_name})
endforeach(input)

foreach(var ${FASTTYPEGEN_${Name}_INPUTS_NOEXT})
	set(FASTTYPEGEN_${Name}_OUTPUTS ${FASTTYPEGEN_${Name}_OUTPUTS} ${CMAKE_CURRENT_BINARY_DIR}/${var}.cpp ${CMAKE_CURRENT_BINARY_DIR}/${var}.h ${CMAKE_CURRENT_BINARY_DIR}/${var}.inl)
endforeach(var)

## Notice that the file copy operator is performed at the time when "cmake" is called and "Makefiles" are generated. 
## Not at the time when "make" is called. 
file(COPY ${ARGN} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

add_custom_command(
  OUTPUT ${FASTTYPEGEN_${Name}_OUTPUTS}
  COMMAND $<TARGET_FILE:fast_type_gen>
  ARGS ${ARGN}
  DEPENDS ${ARGN} fast_type_gen
  COMMENT "[FASTTYPEGEN][${Name}] Building Fast Application Types"
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
  
include_directories(BEFORE ${CMAKE_CURRENT_BINARY_DIR})
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
