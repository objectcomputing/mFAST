
file (GLOB headers "*.h") ## retrieve all header files in current directory
file (GLOB sources "*.cpp")     ## retrieve all source files in current directory



SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

set(mfast_json_SRCS  ${sources} ${headers})

add_library(mfast_json_static STATIC ${mfast_json_SRCS})
target_link_libraries(mfast_json_static mfast_static)

if (UNIX)
  set_target_properties(mfast_json_static PROPERTIES OUTPUT_NAME mfast_json)
endif()
set_target_properties(mfast_json_static PROPERTIES COMPILE_FLAGS -DMFAST_JSON_STATIC_DEFINE)

install(TARGETS mfast_json_static
        EXPORT  mFASTTargets
        RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
        ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib)

set(MFAST_STATIC_LIBRARIES ${MFAST_STATIC_LIBRARIES} mfast_json_static CACHE INTERNAL "")
set(MFAST_STATIC_COMPONENTS ${MFAST_STATIC_COMPONENTS} json_static CACHE INTERNAL "")


if (BUILD_SHARED_LIBS)
  add_library(mfast_json SHARED ${mfast_json_SRCS})

  if (NOT "${CMAKE_VERSION}" VERSION_LESS "2.8.12")
    target_compile_definitions(mfast_json INTERFACE "-DMFAST_DYN_LINK")
  endif (NOT "${CMAKE_VERSION}" VERSION_LESS "2.8.12")

  if (CMAKE_COMPILER_IS_GNUCXX OR ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang"))
    set_target_properties(mfast_json PROPERTIES COMPILE_FLAGS -fvisibility=hidden)
  endif()
  target_link_libraries(mfast_json mfast)
  set_target_properties(mfast_json PROPERTIES
                        VERSION "${MFAST_VERSION}"
                        SOVERSION "${MFAST_SOVERSION}")
  if ("${CMAKE_VERSION}" VERSION_LESS "2.8.12")
    set_target_properties(mfast_json PROPERTIES COMPILE_FLAGS -DMFAST_DYN_LINK)
  endif ("${CMAKE_VERSION}" VERSION_LESS "2.8.12")

  install(TARGETS mfast_json
          EXPORT  mFASTTargets
          RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
          LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib)

  set(MFAST_SHARED_LIBRARIES ${MFAST_SHARED_LIBRARIES} mfast_json CACHE INTERNAL "")
  set(MFAST_DYNAMIC_COMPONENTS ${MFAST_DYNAMIC_COMPONENTS} json CACHE INTERNAL "")
endif (BUILD_SHARED_LIBS)

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        DESTINATION "${INSTALL_INCLUDE_DIR}/mfast"
        FILES_MATCHING PATTERN "*.h")
