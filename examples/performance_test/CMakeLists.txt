
set(TEST_LIBS ${MFAST_LIBRARIES}
              # ${Boost_SYSTEM_LIBRARY}
    )

set_property(
   DIRECTORY
   PROPERTY COMPILE_DEFINITIONS DATA_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/complex30000.dat\" TEMPLATE_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/example.xml\";
   )

add_executable (mf_generic_decode generic_template_test.cpp)
target_link_libraries (mf_generic_decode
    			       # ${Boost_SYSTEM_LIBRARY} # only necessary when boost::chrono is used for measuring time
                       ${TEST_LIBS})

add_executable (mf_generic_decode_encode generic_template_test.cpp)
set_target_properties(mf_generic_decode_encode PROPERTIES COMPILE_FLAGS -DWITH_ENCODE)
target_link_libraries (mf_generic_decode_encode
                      ${TEST_LIBS})

FASTTYPEGEN_TARGET(example example.xml)

add_executable (mf_fixed_decode ${FASTTYPEGEN_example_OUTPUTS} fixed_template_test.cpp)
target_link_libraries (mf_fixed_decode
                       ${TEST_LIBS} )

add_executable (mf_fixed_decode_encode ${FASTTYPEGEN_example_OUTPUTS} fixed_template_test.cpp)
set_target_properties(mf_fixed_decode_encode PROPERTIES COMPILE_FLAGS -DWITH_ENCODE)

target_link_libraries (mf_fixed_decode_encode
                      ${TEST_LIBS} )


add_executable (mf_fixed_decode_v2 ${FASTTYPEGEN_example_OUTPUTS} fixed_template_test_v2.cpp)
target_link_libraries (mf_fixed_decode_v2
                       ${TEST_LIBS} )
