

# function(add_test_executable executable_name)
#   add_executable (${executable_name} ${executable_name}.cpp)
#   target_link_libraries (${executable_name} mfast_static  ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
# endfunction(add_test_executable)




FASTTYPEGEN_TARGET(test_types1 test1.xml test2.xml)
FASTTYPEGEN_TARGET(test_types3 test3.xml)
FASTTYPEGEN_TARGET(test_types4 test4.xml)
FASTTYPEGEN_TARGET(test_types5 test5.xml)
FASTTYPEGEN_TARGET(test_scp scp.xml)


add_executable (mfast_test
                test_main.cpp
                field_ref_test.cpp
                fast_istream_test.cpp
                fast_ostream_test.cpp
                decoder_operator_test.cpp
                encoder_operator_test.cpp
                arena_allocator_test.cpp
                field_comparator_test.cpp
                coder_test.cpp
                value_storage_test.cpp
                ${FASTTYPEGEN_test_types1_OUTPUTS}
                ${FASTTYPEGEN_test_types3_OUTPUTS}
                ${FASTTYPEGEN_test_types4_OUTPUTS}
                ${FASTTYPEGEN_test_types5_OUTPUTS}
                ${FASTTYPEGEN_test_scp_OUTPUTS}
                fast_type_gen_test.cpp
                dictionary_builder_test.cpp
                json_test.cpp
                int_vector_test.cpp
                composite_type_test.cpp
			    aggregate_view_test.cpp)

target_link_libraries (mfast_test
                       mfast_static
                       mfast_coder_static
                       mfast_json_static
					   mfast_xml_parser_static
                       ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})

if (MSVC_IDE)
	add_test(NAME mfast_test
	         COMMAND ${PROJECT_BINARY_DIR}/bin/invoke.bat $<TARGET_FILE:mfast_test>)
else()
	add_test(mfast_test mfast_test)
endif()

