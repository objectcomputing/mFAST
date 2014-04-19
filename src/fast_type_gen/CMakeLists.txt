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
                       ${Boost_SYSTEM_LIBRARY}
                       ${Boost_FILESYSTEM_LIBRARY})

install(TARGETS fast_type_gen
        EXPORT  mFASTTargets
        RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin)