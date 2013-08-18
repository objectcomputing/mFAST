
set(fastxml_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/../../tinyxml2/tinyxml2.cpp ${CMAKE_CURRENT_SOURCE_DIR}/FastXMLVisitor.cpp PARENT_SCOPE)
add_library(fastxml OBJECT ../../tinyxml2/tinyxml2.cpp FastXMLVisitor.cpp)