
file (GLOB headers "*.h") ## retrieve all header files in current directory
file (GLOB sources "*.cpp")     ## retrieve all source files in current directory



SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

set(mfast_json_SRCS  ${sources} ${headers})

add_library(mfast_json_static STATIC ${mfast_json_SRCS})

if (UNIX)
	set_target_properties(mfast_json_static PROPERTIES OUTPUT_NAME mfast_json)
endif()
set_target_properties(mfast_json_static PROPERTIES COMPILE_FLAGS -DMFAST_JSON_STATIC_DEFINE)

install(TARGETS mfast_json_static
        RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
		ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib)


if (BUILD_SHARED_LIBS)
  add_library(mfast_json SHARED ${mfast_json_SRCS})
  if (CMAKE_COMPILER_IS_GNUCXX OR ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang"))
	set_target_properties(mfast_json PROPERTIES COMPILE_FLAGS -fvisibility=hidden)
  endif()
  target_link_libraries(mfast_json mfast)

  install(TARGETS mfast_json
          RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
  	      LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib)
endif()


install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        DESTINATION "${INSTALL_INCLUDE_DIR}"
        FILES_MATCHING PATTERN "*.h")