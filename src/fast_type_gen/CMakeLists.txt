SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

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

install(TARGETS fast_type_gen
        EXPORT  mFASTTargets
        RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin)

set(FAST_TYPE_GEN_INSTALL_LOCATION "${INSTALL_BIN_DIR}/fast_type_gen${CMAKE_EXECUTABLE_SUFFIX}" PARENT_SCOPE)
