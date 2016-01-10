#########################################################################
#  The following lines shows how to write CMakeLists.txt for mFAST applications
#  when the applications are not in the mFAST source tree.
##########################################################################

# cmake_minimum_required(VERSION 2.8)
# find_package(mFAST REQUIRED COMPONENTS coder)
#
# include_directories(${MFAST_INCLUDE_DIR})
# link_directories(${MFAST_LIBRARY_DIRS})


FASTTYPEGEN_TARGET(MDRefreshSample MDRefreshSample.xml)
add_executable (message_printer ${FASTTYPEGEN_MDRefreshSample_OUTPUTS} message_printer.cpp)
target_link_libraries (message_printer ${MFAST_LIBRARIES})
