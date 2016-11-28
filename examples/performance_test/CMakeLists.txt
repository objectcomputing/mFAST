
set(TEST_LIBS ${MFAST_LIBRARIES}
              # ${Boost_SYSTEM_LIBRARY}
    )

set(DATA_FILE "complex30000.dat")
set(TEMPLATE_FILE "example.xml")



if (${CMAKE_SYSTEM_NAME} STREQUAL "Emscripten")
    # SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}" )
    # SET( CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}" )
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
    file(COPY ${DATA_FILE} ${TEMPLATE_FILE} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  --preload-file ${DATA_FILE}  --preload-file ${TEMPLATE_FILE}")
    set_property(
       DIRECTORY
       PROPERTY COMPILE_DEFINITIONS DATA_FILE=\"${DATA_FILE}\" TEMPLATE_FILE=\"${TEMPLATE_FILE}\";
       )
else()
    set_property(
       DIRECTORY
       PROPERTY COMPILE_DEFINITIONS DATA_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/${DATA_FILE}\" TEMPLATE_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/${TEMPLATE_FILE}\";
       )
endif(${CMAKE_SYSTEM_NAME} STREQUAL "Emscripten")

FASTTYPEGEN_TARGET(example example.xml)

add_executable (mf_generic_decode generic_template_test.cpp)
target_link_libraries (mf_generic_decode
    			       # ${Boost_SYSTEM_LIBRARY} # only necessary when boost::chrono is used for measuring time
                       ${TEST_LIBS})

add_executable (mf_generic_decode_encode generic_template_test.cpp)
set_target_properties(mf_generic_decode_encode PROPERTIES COMPILE_FLAGS -DWITH_ENCODE)
target_link_libraries (mf_generic_decode_encode
                      ${TEST_LIBS})


add_executable (mf_fixed_decode ${FASTTYPEGEN_example_OUTPUTS} fixed_template_test.cpp)
target_link_libraries (mf_fixed_decode
                       ${TEST_LIBS} )

add_executable (mf_fixed_decode_encode ${FASTTYPEGEN_example_OUTPUTS} fixed_template_test.cpp)
set_target_properties(mf_fixed_decode_encode PROPERTIES COMPILE_FLAGS -DWITH_ENCODE)
add_dependencies(mf_fixed_decode_encode mf_fixed_decode)

target_link_libraries (mf_fixed_decode_encode
                      ${TEST_LIBS} )


add_executable (mf_fixed_decode_v2 ${FASTTYPEGEN_example_OUTPUTS} fixed_template_test_v2.cpp)
add_dependencies(mf_fixed_decode_v2 mf_fixed_decode_encode)
target_link_libraries (mf_fixed_decode_v2
                       ${TEST_LIBS} )

add_executable (mf_fixed_decode_encode_v2 ${FASTTYPEGEN_example_OUTPUTS} fixed_template_test_v2.cpp)
set_target_properties(mf_fixed_decode_encode_v2 PROPERTIES COMPILE_FLAGS -DWITH_ENCODE)
add_dependencies(mf_fixed_decode_encode_v2 mf_fixed_decode_v2)
target_link_libraries (mf_fixed_decode_encode_v2
                      ${TEST_LIBS} )
