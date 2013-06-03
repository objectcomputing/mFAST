

add_executable (fast_type_gen
	            fast_type_gen.cpp
				../fastxml/tinyxml2.cpp
				../fastxml/FastXMLVisitor.cpp
				FastXML2Header.cpp
				FastXML2Inline.cpp
				FastXML2Source.cpp )

target_link_libraries (fast_type_gen ${Boost_SYSTEM_LIBRARY} ${Boost_FILESYSTEM_LIBRARY})
