# see https://cmake.org/cmake/help/book/mastering-cmake/chapter/Cross%20Compiling%20With%20CMake.html#running-executables-built-in-the-project

if (CMAKE_CROSSCOMPILING)
   # import fast_type_gen from the binary tree of a previous host-build, to support generating code when cross-compiling
   find_package(mFASTTypeGen)
else()
   file (GLOB headers CONFIGURE_DEPENDS "*.h") ## retrieve all header files in current directory
   file (GLOB sources CONFIGURE_DEPENDS "*.cpp")     ## retrieve all source files in current directory

   add_executable (fast_type_gen)
   target_sources(fast_type_gen PRIVATE ${headers} ${sources})

   target_link_libraries (fast_type_gen PRIVATE
                   mfast_xml_parser_static
                   mfast_static
                   mfast_coder_static)

   set_target_properties(fast_type_gen PROPERTIES VERSION ${mFAST_VERSION})

   install(TARGETS fast_type_gen
           EXPORT mFASTTargets)

   add_custom_target(install.fast_type_gen
                   COMMAND ${CMAKE_COMMAND} -DBUILD_TYPE=${CMAKE_BUILD_TYPE} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_install.cmake
                   DEPENDS fast_type_gen)

   # export the host-built fast_type_gen to the binary tree, to be used from another cross-compiling build
   export(TARGETS fast_type_gen FILE ${CMAKE_BINARY_DIR}/mFASTTypeGenConfig.cmake)
endif()