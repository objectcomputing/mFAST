

# function(add_test_executable executable_name)
#   add_executable (${executable_name} ${executable_name}.cpp)
#   target_link_libraries (${executable_name} mfast_static  ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
# endfunction(add_test_executable)
# Includes Catch in the project:
#add_subdirectory(${EXT_PROJECTS_DIR}/catch)
include_directories(${PROJECT_SOURCE_DIR}/catch/include)


FASTTYPEGEN_TARGET(simple_types1 simple1.xml)
FASTTYPEGEN_TARGET(simple_types2 simple2.xml)
FASTTYPEGEN_TARGET(simple_types3 simple3.xml)
FASTTYPEGEN_TARGET(simple_types4 simple4.xml)
FASTTYPEGEN_TARGET(simple_types5 simple5.xml)
FASTTYPEGEN_TARGET(simple_types6 simple6.xml)
FASTTYPEGEN_TARGET(simple_types7 simple7.xml)
FASTTYPEGEN_TARGET(simple_types8 simple8.xml)

FASTTYPEGEN_TARGET(test_types1 test1.xml test2.xml)
FASTTYPEGEN_TARGET(test_types3 test3.xml)
FASTTYPEGEN_TARGET(test_types4 test4.xml)
FASTTYPEGEN_TARGET(test_types5 test5.xml)
FASTTYPEGEN_TARGET(test_scp scp.xml)

add_executable (mfast_test
                test_main.cpp
                arena_allocator_test.cpp
                field_ref_test.cpp
                fast_istream_test.cpp
                fast_ostream_test.cpp
                decoder_operator_test.cpp
                encoder_operator_test.cpp
                field_comparator_test.cpp
                coder_test.cpp
                value_storage_test.cpp
                ${FASTTYPEGEN_test_types1_OUTPUTS}
                ${FASTTYPEGEN_test_types3_OUTPUTS}
                ${FASTTYPEGEN_test_types4_OUTPUTS}
                ${FASTTYPEGEN_test_types5_OUTPUTS}
                ${FASTTYPEGEN_test_scp_OUTPUTS}
                ${FASTTYPEGEN_simple_types1_OUTPUTS}
                ${FASTTYPEGEN_simple_types2_OUTPUTS}
                ${FASTTYPEGEN_simple_types3_OUTPUTS}
                ${FASTTYPEGEN_simple_types4_OUTPUTS}
                ${FASTTYPEGEN_simple_types5_OUTPUTS}
                ${FASTTYPEGEN_simple_types6_OUTPUTS}
                ${FASTTYPEGEN_simple_types7_OUTPUTS}
                ${FASTTYPEGEN_simple_types8_OUTPUTS}
                fast_type_gen_test.cpp
                dictionary_builder_test.cpp
                json_test.cpp
                int_vector_test.cpp
                composite_type_test.cpp
                aggregate_view_test.cpp
                simple_coder_test.cpp
                scp_reset_test.cpp
            )

target_link_libraries (mfast_test
                       mfast_static
                       mfast_coder_static
                       mfast_json_static
                       mfast_xml_parser_static)


if(${CMAKE_SYSTEM_NAME} STREQUAL "Emscripten")
    add_test(mfast_test node mfast_test.js)
else()
    add_test(mfast_test mfast_test)
endif()


# add_dependencies(mfast_test catch)

