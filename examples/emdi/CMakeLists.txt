# cmake_minimum_required(VERSION 2.8)
# project (emdi)
#
# find_package(mFAST REQUIRED COMPONENTS coder)
# find_package(Boost REQUIRED COMPONENTS system filesystem )
#
# include_directories(${MFAST_INCLUDE_DIR} )
# link_directories(${MFAST_LIBRARY_DIRS})

fasttypegen_target(emdi_snapshot emdi_snapshot.xml scp.xml)

add_executable(emdi ${FASTTYPEGEN_emdi_snapshot_OUTPUTS} fixed_template_test)
target_link_libraries(emdi ${MFAST_LIBRARIES})
