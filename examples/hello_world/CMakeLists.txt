#########################################################################
#  The following lines shows how to write CMakeLists.txt for mFAST applications
#  when the applications are not in the mFAST source tree.
##########################################################################

# cmake_minimum_required(VERSION 2.8)
# find_package(mFAST REQUIRED COMPONENTS coder)
#
# include_directories(${MFAST_INCLUDE_DIR})
# link_directories(${MFAST_LIBRARY_DIRS})

add_executable (hello_world hello_world.cpp)
target_link_libraries (hello_world mfast_coder_static mfast_json_static mfast_xml_parser_static)
