file (GLOB_RECURSE headers CONFIGURE_DEPENDS "*.h") ## retrieve all header files in current directory
file (GLOB_RECURSE sources CONFIGURE_DEPENDS "*.cpp")     ## retrieve all source files in current directory

set(TINYXML2_ROOT ${PROJECT_SOURCE_DIR}/tinyxml2/)

# create our own tinyxml2 target, easier to integrate in the build
add_library(tinyxml2_static OBJECT)
target_sources(tinyxml2_static PRIVATE ${TINYXML2_ROOT}/tinyxml2.cpp)
target_sources(tinyxml2_static PUBLIC FILE_SET HEADERS BASE_DIRS ${TINYXML2_ROOT} FILES ${TINYXML2_ROOT}/tinyxml2.h)

install(TARGETS tinyxml2_static
        EXPORT  mFASTTargets
        FILE_SET HEADERS DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/mfast/xml_parser)

add_library(mfast_xml_parser_static STATIC)
target_sources(mfast_xml_parser_static PRIVATE ${sources})
target_sources(mfast_xml_parser_static PUBLIC FILE_SET HEADERS BASE_DIRS ${PROJECT_SOURCE_DIR}/src FILES ${headers})
target_link_libraries(mfast_xml_parser_static PUBLIC mfast_static tinyxml2_static)
target_compile_definitions(mfast_xml_parser_static PUBLIC MFAST_CODER_STATIC_DEFINE)

if (UNIX)
  set_target_properties(mfast_xml_parser_static PROPERTIES OUTPUT_NAME mfast_xml_parser)
endif()

install(TARGETS mfast_xml_parser_static 
        EXPORT  mFASTTargets
        FILE_SET HEADERS)

if (BUILD_SHARED_LIBS)
  add_library(tinyxml2_shared OBJECT)
  target_sources(tinyxml2_shared PRIVATE ${TINYXML2_ROOT}/tinyxml2.cpp)
  target_sources(tinyxml2_shared PUBLIC FILE_SET HEADERS BASE_DIRS ${TINYXML2_ROOT} FILES ${TINYXML2_ROOT}/tinyxml2.h)
  set_property(TARGET tinyxml2_shared PROPERTY POSITION_INDEPENDENT_CODE ON)

  add_library(mfast_xml_parser SHARED)
  target_sources(mfast_xml_parser PRIVATE ${sources})
  target_sources(mfast_xml_parser PUBLIC FILE_SET HEADERS BASE_DIRS ${PROJECT_SOURCE_DIR}/src FILES ${headers})
  target_compile_definitions(mfast_xml_parser INTERFACE MFAST_DYN_LINK)
  target_link_libraries(mfast_xml_parser PUBLIC mfast PRIVATE tinyxml2_shared)
  set_property(TARGET mfast_xml_parser PROPERTY VERSION ${MFAST_VERSION})
  set_property(TARGET mfast_xml_parser PROPERTY SOVERSION ${MFAST_VERSION})

  install(TARGETS mfast_xml_parser
          EXPORT  mFASTTargets
          FILE_SET HEADERS)
endif (BUILD_SHARED_LIBS)