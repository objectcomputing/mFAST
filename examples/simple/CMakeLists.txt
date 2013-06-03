
FASTTYPEGEN_TARGET(MDRefreshSample MDRefreshSample.xml)
add_executable (simple_test ${FASTTYPEGEN_MDRefreshSample_OUTPUTS} simple_test.cpp)
target_link_libraries (simple_test ${MFAST_LIBRARIES})
