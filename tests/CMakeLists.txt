

# function(add_test_executable executable_name)
# 	add_executable (${executable_name} ${executable_name}.cpp)
# 	target_link_libraries (${executable_name} mfast_static  ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
# endfunction(add_test_executable)




FASTTYPEGEN_TARGET(test_types test1.xml test2.xml)


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
			    ${FASTTYPEGEN_test_types_OUTPUTS}
			    fast_type_gen_test.cpp
			    dictionary_builder_test.cpp)

target_link_libraries (mfast_test mfast_static  ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})



