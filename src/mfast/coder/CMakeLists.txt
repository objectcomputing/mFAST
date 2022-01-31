
file (GLOB_RECURSE headers "*.h") ## retrieve all header files in current directory

set(sources common/dictionary_builder.cpp
            decoder/decoder_field_operator.cpp
            decoder/decoder_presence_map.cpp
            decoder/fast_decoder.cpp
            decoder/fast_istream.cpp
            encoder/encoder_field_operator.cpp
            encoder/fast_encoder.cpp
            encoder/fast_ostreambuf.cpp
            encoder_v2/fast_encoder_core.cpp
)


SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

set(mfast_coder_SRCS ${sources} ${headers})

add_library(mfast_coder_static STATIC ${mfast_coder_SRCS})
target_link_libraries(mfast_coder_static mfast_static)

if (UNIX)
  set_target_properties(mfast_coder_static PROPERTIES OUTPUT_NAME mfast_coder)
endif()
set_target_properties(mfast_coder_static PROPERTIES COMPILE_FLAGS -DMFAST_CODER_STATIC_DEFINE)

install(TARGETS mfast_coder_static
        EXPORT  mFASTTargets
        RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
        ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib)

set(MFAST_STATIC_LIBRARIES ${MFAST_STATIC_LIBRARIES} mfast_coder_static CACHE INTERNAL "")
set(MFAST_STATIC_COMPONENTS ${MFAST_STATIC_COMPONENTS} coder_static CACHE INTERNAL "")


if (BUILD_SHARED_LIBS)
  add_library(mfast_coder SHARED ${mfast_coder_SRCS} decoder_v2/fast_decoder_core.cpp)

  if (NOT "${CMAKE_VERSION}" VERSION_LESS "2.8.12")
    target_compile_definitions(mfast_coder INTERFACE "-DMFAST_DYN_LINK")
  endif (NOT "${CMAKE_VERSION}" VERSION_LESS "2.8.12")

  if (CMAKE_COMPILER_IS_GNUCXX OR ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang"))
    set_target_properties(mfast_coder PROPERTIES COMPILE_FLAGS -fvisibility=hidden)
  endif()
  target_link_libraries(mfast_coder mfast)
  set_target_properties(mfast_coder PROPERTIES
                        VERSION "${MFAST_VERSION}"
                        SOVERSION "${MFAST_SOVERSION}")
  if ("${CMAKE_VERSION}" VERSION_LESS "2.8.12")
    set_target_properties(mfast_coder PROPERTIES COMPILE_FLAGS -DMFAST_DYN_LINK)
  endif ("${CMAKE_VERSION}" VERSION_LESS "2.8.12")

  install(TARGETS mfast_coder
          EXPORT  mFASTTargets
          RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
          ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib
          LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib)

  set(MFAST_SHARED_LIBRARIES ${MFAST_SHARED_LIBRARIES} mfast_coder CACHE INTERNAL "")
  set(MFAST_DYNAMIC_COMPONENTS ${MFAST_DYNAMIC_COMPONENTS} coder CACHE INTERNAL "")
endif (BUILD_SHARED_LIBS)

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        DESTINATION "${INSTALL_INCLUDE_DIR}/mfast"
        FILES_MATCHING PATTERN "*.h")
