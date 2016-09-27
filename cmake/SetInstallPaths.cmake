
# Offer the user the choice of overriding the installation directories
set(INSTALL_LIB_DIR lib CACHE PATH "Installation directory for libraries")
set(INSTALL_BIN_DIR bin CACHE PATH "Installation directory for executables")
set(INSTALL_INCLUDE_DIR include CACHE PATH "Installation directory for header files")
set(INSTALL_DATA_DIR share CACHE PATH "Installation directory for data files")

# determine default path for CMake files
if(WIN32 AND NOT CYGWIN)
  set(DEF_INSTALL_CMAKE_DIR CMake)
else()
  if(${CMAKE_LIBRARY_ARCHITECTURE}) #support for multiarch debian-based distros, need cmake>=2.8.5
    set(DEF_INSTALL_CMAKE_DIR     lib/${CMAKE_LIBRARY_ARCHITECTURE}/cmake/mFAST)
  else()
    set(DEF_INSTALL_CMAKE_DIR     lib/cmake/mFAST)
  endif()
endif()
set(INSTALL_CMAKE_DIR ${DEF_INSTALL_CMAKE_DIR} CACHE PATH
"Installation directory for CMake files")

# Make relative paths absolute (needed later on)
foreach (p IN ITEMS LIB BIN INCLUDE DATA CMAKE)
  set (var "INSTALL_${p}_DIR")
  if (NOT IS_ABSOLUTE "${${var}}")
    set("ABSOLUTE_${var}" "${CMAKE_INSTALL_PREFIX}/${${var}}")
  else ()
    set("ABSOLUTE_${var}" "${${var}}")
  endif ()
endforeach (p)