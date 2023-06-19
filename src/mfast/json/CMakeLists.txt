file (GLOB headers CONFIGURE_DEPENDS "*.h") ## retrieve all header files in current directory
file (GLOB sources CONFIGURE_DEPENDS "*.cpp")     ## retrieve all source files in current directory

set(mfast_json_SRCS  ${sources} ${headers})

add_library(mfast_json_static STATIC)
target_sources(mfast_json_static PRIVATE ${sources})
target_sources(mfast_json_static PUBLIC FILE_SET HEADERS BASE_DIRS ${PROJECT_SOURCE_DIR}/src FILES ${headers})
target_link_libraries(mfast_json_static PUBLIC mfast_static)
target_compile_definitions(mfast_json_static PUBLIC MFAST_JSON_STATIC_DEFINE)

if (UNIX)
  set_target_properties(mfast_json_static PROPERTIES OUTPUT_NAME mfast_json)
endif()

install(TARGETS mfast_json_static 
        EXPORT  mFASTTargets
        FILE_SET HEADERS)

if (BUILD_SHARED_LIBS)
  add_library(mfast_json SHARED)
  target_sources(mfast_json PRIVATE ${sources})
  target_sources(mfast_json PUBLIC FILE_SET HEADERS BASE_DIRS ${PROJECT_SOURCE_DIR}/src FILES ${headers})
  target_link_libraries(mfast_json PUBLIC mfast)
  target_compile_definitions(mfast_json INTERFACE MFAST_DYN_LINK)
  set_property(TARGET mfast_json PROPERTY VERSION ${MFAST_VERSION})
  set_property(TARGET mfast_json PROPERTY SOVERSION ${MFAST_VERSION})

  install(TARGETS mfast_json
          EXPORT  mFASTTargets
          FILE_SET HEADERS)
endif (BUILD_SHARED_LIBS)