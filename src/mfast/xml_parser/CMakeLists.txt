
file (GLOB_RECURSE headers "*.h") ## retrieve all header files in current directory
file (GLOB_RECURSE sources "*.cpp")     ## retrieve all source files in current directory



SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

include_directories("${CMAKE_SOURCE_DIR}/tinyxml2")

set(mfast_xml_parser_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/../../../tinyxml2/tinyxml2.cpp ${sources} ${headers})

add_library(mfast_xml_parser_static STATIC ${mfast_xml_parser_SRCS})

if (UNIX)
	set_target_properties(mfast_xml_parser_static PROPERTIES OUTPUT_NAME mfast_xml_parser)
endif()
set_target_properties(mfast_xml_parser_static PROPERTIES COMPILE_FLAGS -DMFAST_CODER_STATIC_DEFINE)

install(TARGETS mfast_xml_parser_static
        EXPORT  mFASTTargets
        RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
		ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib)


if (BUILD_SHARED_LIBS)
  add_library(mfast_xml_parser SHARED ${mfast_xml_parser_SRCS})
  # the add_dependencies is necessary for CMake 2.6.4. Otherwise, the shared
  # library would be built before the static library, and the shared library
  # would be deleted when the static library is built. For CMake 2.8.x, there
  # is no such problem.
  add_dependencies(mfast_xml_parser mfast_xml_parser_static)

  if (CMAKE_COMPILER_IS_GNUCXX OR ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang"))
	set_target_properties(mfast_xml_parser PROPERTIES COMPILE_FLAGS -fvisibility=hidden)
  endif()
  target_link_libraries(mfast_xml_parser mfast)
  set_target_properties(mfast_xml_parser PROPERTIES COMPILE_FLAGS -DMFAST_DYN_LINK)


  install(TARGETS mfast_xml_parser
          EXPORT  mFASTTargets
          RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
  	      LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib)

  SET(MFAST_DYNAMIC_COMPONENTS ${MFAST_DYNAMIC_COMPONENTS} coder CACHE INTERNAL "")
endif()

SET(MFAST_STATIC_COMPONENTS ${MFAST_STATIC_COMPONENTS} coder_static CACHE INTERNAL "")

install(FILES mfast_xml_parser_export.h dynamic_templates_description.h
        DESTINATION "${INSTALL_INCLUDE_DIR}/mfast/xml_parser")

