file (GLOB headers "*.h") ## retrieve all header files in current directory
file (GLOB sources "*.cpp")     ## retrieve all source files in current directory

SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

set(mfast_sqlite3_SRCS  ${sources} ${headers})

add_library(mfast_sqlite3_static STATIC ${mfast_sqlite3_SRCS})

if (UNIX)
	set_target_properties(mfast_sqlite3_static PROPERTIES OUTPUT_NAME mfast_sqlite3)
endif()
set_target_properties(mfast_sqlite3_static PROPERTIES COMPILE_FLAGS -DMFAST_SQLITE3_STATIC_DEFINE)

install(TARGETS mfast_sqlite3_static
        EXPORT  mFASTTargets
        RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
		ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib
        OPTIONAL)


if (BUILD_SHARED_LIBS)
  add_library(mfast_sqlite3 SHARED ${mfast_sqlite3_SRCS})
  # the add_dependencies is necessary for CMake 2.6.4. Otherwise, the shared
  # library would be built before the static library, and the shared library
  # would be deleted when the static library is built. For CMake 2.8.x, there
  # is no such problem.
  add_dependencies(mfast_sqlite3 mfast_sqlite3_static)

  if (CMAKE_COMPILER_IS_GNUCXX OR ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang"))
	set_target_properties(mfast_sqlite3 PROPERTIES COMPILE_FLAGS -fvisibility=hidden)
  endif()
  target_link_libraries(mfast_sqlite3 mfast sqlite3)
  set_target_properties(mfast_sqlite3 PROPERTIES COMPILE_FLAGS -DMFAST_DYN_LINK)


  install(TARGETS mfast_sqlite3
          EXPORT  mFASTTargets
          RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
  	      LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib
          OPTIONAL)

  SET(MFAST_DYNAMIC_COMPONENTS ${MFAST_DYNAMIC_COMPONENTS} sqlite3 CACHE INTERNAL "")
endif()

SET(MFAST_STATIC_COMPONENTS ${MFAST_STATIC_COMPONENTS} sqlite3_static CACHE INTERNAL "")
install(FILES mfast_sqlite3_export.h converter.h field_masks.h
        DESTINATION "${INSTALL_INCLUDE_DIR}/mfast/sqlite3"
        OPTIONAL)
