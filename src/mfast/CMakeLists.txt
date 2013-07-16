file (GLOB_RECURSE headers "*.h") ## retrieve all header files in current directory
file (GLOB_RECURSE sources "*.cpp")     ## retrieve all source files in current directory


SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

set(mfast_SRCS ../fastxml/tinyxml2.cpp ../fastxml/FastXMLVisitor.cpp ${sources} ${headers})

add_library(mfast ${mfast_SRCS})

# set_target_properties(mfast PROPERTIES
#   PUBLIC_HEADER ${headers})

install(TARGETS mfast
        RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
        ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib)

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        DESTINATION "${INSTALL_INCLUDE_DIR}"
        FILES_MATCHING PATTERN "*.h")



