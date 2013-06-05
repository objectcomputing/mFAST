
FASTTYPEGEN_TARGET(MDRefreshSample MDRefreshSample.xml)
add_executable (message_printer ${FASTTYPEGEN_MDRefreshSample_OUTPUTS} message_printer.cpp)
target_link_libraries (message_printer ${MFAST_LIBRARIES})
