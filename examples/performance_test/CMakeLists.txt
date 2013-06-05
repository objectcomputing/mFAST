
add_executable (generic_template_test generic_template_test.cpp)
target_link_libraries (generic_template_test
    			       # ${Boost_SYSTEM_LIBRARY} # only necessary when boost::chrono is used for measuring time
                       ${MFAST_LIBRARIES})

FASTTYPEGEN_TARGET(example example.xml)
add_executable (fixed_template_test ${FASTTYPEGEN_example_OUTPUTS} fixed_template_test.cpp)
target_link_libraries (fixed_template_test
	                   # ${Boost_SYSTEM_LIBRARY} # only necessary when boost::chrono is used for measuring time
				       ${MFAST_LIBRARIES} )
