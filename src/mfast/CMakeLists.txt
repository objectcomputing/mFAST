
file (GLOB headers CONFIGURE_DEPENDS "*.h" ) ## retrieve all header files in current directory
file (GLOB sources CONFIGURE_DEPENDS "*.cpp" )     ## retrieve all source files in current directory

file (GLOB instruction_headers CONFIGURE_DEPENDS "instructions/*.h") ## retrieve all header files in instructions directory
file (GLOB instruction_sources CONFIGURE_DEPENDS "instructions/*.cpp")    ## retrieve all source files in instructions directory

list(APPEND headers ../mfast.h)

add_library(mfast_static STATIC)
target_sources(mfast_static PRIVATE ${sources} ${instruction_sources})
target_sources(mfast_static PUBLIC FILE_SET HEADERS BASE_DIRS ${PROJECT_SOURCE_DIR}/src FILES ${headers} ${instruction_headers})
target_compile_definitions(mfast_static PUBLIC MFAST_STATIC_DEFINE)
target_compile_features(mfast_static PUBLIC cxx_std_14)
target_link_libraries(mfast_static PUBLIC Boost::headers)

if (UNIX)
  set_target_properties(mfast_static PROPERTIES OUTPUT_NAME mfast)
endif()

install(TARGETS mfast_static 
        EXPORT  mFASTTargets
        FILE_SET HEADERS)

if (BUILD_SHARED_LIBS)
  add_library(mfast SHARED)
  target_sources(mfast PRIVATE ${sources} ${instruction_sources})
  target_sources(mfast PUBLIC FILE_SET HEADERS BASE_DIRS ${PROJECT_SOURCE_DIR}/src FILES ${headers} ${instruction_headers})
  target_compile_features(mfast PUBLIC cxx_std_14)
  target_compile_definitions(mfast INTERFACE MFAST_DYN_LINK)
  target_link_libraries(mfast PUBLIC Boost::headers)
  set_property(TARGET mfast PROPERTY VERSION ${MFAST_VERSION})
  set_property(TARGET mfast PROPERTY SOVERSION ${MFAST_VERSION})

  install(TARGETS mfast
          EXPORT  mFASTTargets
          FILE_SET HEADERS)
endif ()

add_subdirectory (coder)
add_subdirectory (xml_parser)
add_subdirectory (json)

if (BUILD_SQLITE3)
    add_subdirectory (sqlite3)
endif(BUILD_SQLITE3)