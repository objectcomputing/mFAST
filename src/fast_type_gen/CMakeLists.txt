
if (NOT CMAKE_CROSSCOMPILING)
  set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
  add_executable (fast_type_gen
                  fast_type_gen.cpp
                  codegen_base.cpp
                  hpp_gen.cpp
                  inl_gen.cpp
                  cpp_gen.cpp)

  target_link_libraries (fast_type_gen
                         mfast_xml_parser_static
                         mfast_static
                         mfast_coder_static)

  set_target_properties(fast_type_gen PROPERTIES VERSION "${MFAST_VERSION}")

  install(TARGETS fast_type_gen
          EXPORT  mFASTTargets
          RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin)
  add_custom_target(install.fast_type_gen
                    COMMAND ${CMAKE_COMMAND} -DBUILD_TYPE=${CMAKE_BUILD_TYPE} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_install.cmake
                    DEPENDS fast_type_gen)
  set(FAST_TYPE_GEN_INSTALL_LOCATION "${INSTALL_BIN_DIR}/fast_type_gen${CMAKE_EXECUTABLE_SUFFIX}" PARENT_SCOPE)
else()
  include(ExternalProject)
  externalproject_add(native_mfast
                      SOURCE_DIR "${PROJECT_SOURCE_DIR}"
                      CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX=${PROJECT_BINARY_DIR}" "-DCMAKE_BUILD_TYPE=RELEASE"
                      BUILD_COMMAND ${CMAKE_COMMAND} --build . --target install.fast_type_gen
                      BUILD_BYPRODUCTS ${PROJECT_BINARY_DIR}/bin/fast_type_gen
                      INSTALL_COMMAND "")
  
  add_executable(fast_type_gen IMPORTED GLOBAL)
  set_property(TARGET fast_type_gen PROPERTY IMPORTED_LOCATION ${PROJECT_BINARY_DIR}/bin/fast_type_gen)
  add_dependencies(fast_type_gen native_mfast)
  
endif()

  