
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

add_executable (fast_type_gen
	            fast_type_gen.cpp
				FastXML2Header.cpp
				FastXML2Inline.cpp
				FastXML2Source.cpp 
			    $<TARGET_OBJECTS:fastxml>)

target_link_libraries (fast_type_gen  ${Boost_SYSTEM_LIBRARY} ${Boost_FILESYSTEM_LIBRARY})


install(TARGETS fast_type_gen
        RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin)