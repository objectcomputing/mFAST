
file (GLOB_RECURSE headers "*.h") ## retrieve all header files in current directory
file (GLOB_RECURSE sources "*.cpp")     ## retrieve all source files in current directory



set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

include_directories("${CMAKE_SOURCE_DIR}/tinyxml2")

set(mfast_xml_parser_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/../../../tinyxml2/tinyxml2.cpp ${sources} ${headers})

add_library(mfast_xml_parser_static STATIC ${mfast_xml_parser_SRCS})
target_link_libraries(mfast_xml_parser_static mfast_static)

if (UNIX)
  set_target_properties(mfast_xml_parser_static PROPERTIES OUTPUT_NAME mfast_xml_parser)
endif()
set_target_properties(mfast_xml_parser_static PROPERTIES COMPILE_FLAGS -DMFAST_CODER_STATIC_DEFINE)
set_target_properties(mfast_xml_parser_static PROPERTIES LINKER_LANGUAGE CXX)

install(TARGETS mfast_xml_parser_static
        EXPORT  mFASTTargets
        RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
        ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib)

set(MFAST_STATIC_LIBRARIES ${MFAST_STATIC_LIBRARIES} mfast_xml_parser_static CACHE INTERNAL "")
set(MFAST_STATIC_COMPONENTS ${MFAST_STATIC_COMPONENTS} xml_parser_static CACHE INTERNAL "")


if (BUILD_SHARED_LIBS)
  add_library(mfast_xml_parser SHARED ${mfast_xml_parser_SRCS})

  if (NOT "${CMAKE_VERSION}" VERSION_LESS "2.8.12")
    target_compile_definitions(mfast_xml_parser INTERFACE "-DMFAST_DYN_LINK")
  endif (NOT "${CMAKE_VERSION}" VERSION_LESS "2.8.12")

  if (CMAKE_COMPILER_IS_GNUCXX OR ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang"))
    set_target_properties(mfast_xml_parser PROPERTIES COMPILE_FLAGS -fvisibility=hidden)
  endif()

  target_link_libraries(mfast_xml_parser mfast)
  set_target_properties(mfast_xml_parser PROPERTIES
                        LINKER_LANGUAGE CXX
                        VERSION "${MFAST_VERSION}"
                        SOVERSION "${MFAST_SOVERSION}")
  if ("${CMAKE_VERSION}" VERSION_LESS "2.8.12")
    set_target_properties(mfast_xml_parser PROPERTIES COMPILE_FLAGS -DMFAST_DYN_LINK)
  endif ("${CMAKE_VERSION}" VERSION_LESS "2.8.12")

  install(TARGETS mfast_xml_parser
          EXPORT  mFASTTargets
          RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
          LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib)

  set(MFAST_SHARED_LIBRARIES ${MFAST_SHARED_LIBRARIES} mfast_xml_parser CACHE INTERNAL "")
  set(MFAST_DYNAMIC_COMPONENTS ${MFAST_DYNAMIC_COMPONENTS} xml_parser CACHE INTERNAL "")
endif (BUILD_SHARED_LIBS)

install(FILES mfast_xml_parser_export.h dynamic_templates_description.h
        DESTINATION "${INSTALL_INCLUDE_DIR}/mfast/xml_parser")

