
file (GLOB_RECURSE headers CONFIGURE_DEPENDS "*.h") ## retrieve all header files in current directory

set(sources common/dictionary_builder.cpp
            decoder/decoder_field_operator.cpp
            decoder/decoder_presence_map.cpp
            decoder/fast_decoder.cpp
            decoder/fast_istream.cpp
            encoder/encoder_field_operator.cpp
            encoder/fast_encoder.cpp
            encoder/fast_ostreambuf.cpp
            encoder_v2/fast_encoder_core.cpp)

add_library(mfast_coder_static STATIC)
target_sources(mfast_coder_static PRIVATE ${sources})
target_sources(mfast_coder_static PUBLIC FILE_SET HEADERS BASE_DIRS ${PROJECT_SOURCE_DIR}/src/ FILES ${headers})
target_link_libraries(mfast_coder_static PUBLIC mfast_static)
target_compile_definitions(mfast_coder_static PUBLIC MFAST_CODER_STATIC_DEFINE)

if (UNIX)
  set_target_properties(mfast_coder_static PROPERTIES OUTPUT_NAME mfast_coder)
endif()

install(TARGETS mfast_coder_static
        EXPORT  mFASTTargets
        FILE_SET HEADERS)

if (BUILD_SHARED_LIBS)
  add_library(mfast_coder SHARED)
  target_sources(mfast_coder PRIVATE ${sources} decoder_v2/fast_decoder_core.cpp)
  target_sources(mfast_coder PUBLIC FILE_SET HEADERS BASE_DIRS ${PROJECT_SOURCE_DIR}/src/ FILES ${headers})
  target_compile_definitions(mfast_coder INTERFACE MFAST_DYN_LINK)
  target_link_libraries(mfast_coder PUBLIC mfast)
  set_property(TARGET mfast_coder PROPERTY VERSION ${MFAST_VERSION})
  set_property(TARGET mfast_coder PROPERTY SOVERSION ${MFAST_VERSION})

  install(TARGETS mfast_coder
          EXPORT  mFASTTargets
          FILE_SET HEADERS)

endif (BUILD_SHARED_LIBS)