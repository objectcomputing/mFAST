
file (GLOB headers "*.h") ## retrieve all header files in current directory
file (GLOB sources "*.cpp")     ## retrieve all source files in current directory

file (GLOB instruction_headers "instructions/*.h") ## retrieve all header files in instructions directory
file (GLOB instruction_sources "instructions/*.cpp")     ## retrieve all source files in instructions directory

SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

set(mfast_SRCS  ${sources} ${instruction_sources} ${headers} ${instruction_headers})

add_library(mfast_static STATIC ${mfast_SRCS})


if (UNIX)
	set_target_properties(mfast_static PROPERTIES OUTPUT_NAME mfast)
endif()
set_target_properties(mfast_static PROPERTIES COMPILE_FLAGS -DMFAST_STATIC_DEFINE)

install(TARGETS mfast_static
        EXPORT  mFASTTargets
        RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
		ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib)


if (BUILD_SHARED_LIBS)
  add_library(mfast SHARED ${mfast_SRCS})
  # the add_dependencies is necessary for CMake 2.6.4. Otherwise, the shared
  # library would be built before the static library, and the shared library
  # would be deleted when the static library is built. For CMake 2.8.x, there
  # is no such problem.
  add_dependencies(mfast mfast_static)

  if (CMAKE_COMPILER_IS_GNUCXX OR ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang"))
	set_target_properties(mfast PROPERTIES COMPILE_FLAGS -fvisibility=hidden)
  endif()

  install(TARGETS mfast
          EXPORT  mFASTTargets
          RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
  	      LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib)

  SET(MFAST_LIBRARY mfast CACHE INTERNAL "mFast Library name")
else()
  SET(MFAST_LIBRARY mfast_static CACHE INTERNAL "mFast Library name")
endif()

add_subdirectory (coder)
add_subdirectory (xml_parser)
add_subdirectory (json)

if (BUILD_SQLITE3)
    add_subdirectory (sqlite3)
endif(BUILD_SQLITE3)

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        DESTINATION "${INSTALL_INCLUDE_DIR}"
        FILES_MATCHING PATTERN "*.h")
