find_package(SQLite3 REQUIRED)

file (GLOB headers CONFIGURE_DEPENDS "*.h") ## retrieve all header files in current directory
file (GLOB sources CONFIGURE_DEPENDS "*.cpp")     ## retrieve all source files in current directory

add_library(mfast_sqlite3_static STATIC)
target_sources(mfast_sqlite3_static PRIVATE ${sources})
target_sources(mfast_sqlite3_static PUBLIC FILE_SET HEADERS BASE_DIRS ${PROJECT_SOURCE_DIR}/src/ FILES ${headers})
target_compile_definitions(mfast_sqlite3_static PUBLIC MFAST_SQLITE3_STATIC_DEFINE)
target_link_libraries(mfast_sqlite3_static mfast_static SQLite::SQLite3)

if (UNIX)
  set_target_properties(mfast_sqlite3_static PROPERTIES OUTPUT_NAME mfast_sqlite3)
endif()

install(TARGETS mfast_sqlite3_static
        EXPORT  mFASTTargets
        FILE_SET HEADERS
        OPTIONAL)

if (BUILD_SHARED_LIBS)
  add_library(mfast_sqlite3 SHARED)
  target_sources(mfast_sqlite3 PRIVATE ${sources})
  target_sources(mfast_sqlite3 PUBLIC FILE_SET HEADERS BASE_DIRS ${PROJECT_SOURCE_DIR}/src/ FILES ${headers})
  target_compile_definitions(mfast_sqlite3 PUBLIC MFAST_DYN_LINK)
  target_link_libraries(mfast_sqlite3 mfast SQLite::SQLite3)
  set_property(TARGET mfast_sqlite3 PROPERTY VERSION ${MFAST_VERSION})
  set_property(TARGET mfast_sqlite3 PROPERTY SOVERSION ${MFAST_VERSION})

  install(TARGETS mfast_sqlite3
          EXPORT  mFASTTargets
          FILE_SET HEADERS
          OPTIONAL)
endif (BUILD_SHARED_LIBS)