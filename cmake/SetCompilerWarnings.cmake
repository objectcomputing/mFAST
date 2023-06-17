# Initialize CXXFLAGS.
add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/W4>
                    $<$<CXX_COMPILER_ID:GNU,Clang>:-W>
                    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall>)

add_compile_definitions($<$<CXX_COMPILER_ID:MSVC>:_SCL_SECURE_NO_WARNINGS> 
                        $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>)