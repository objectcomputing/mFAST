# Provide mfast-config.cmake and mfast-config.version to be used by other applications
# ===============================

export(PACKAGE ${CMAKE_PROJECT_NAME})

if (CMAKE_CROSSCOMPILING)
  export(TARGETS ${MFAST_SHARED_LIBRARIES} ${MFAST_STATIC_LIBRARIES}
         FILE "${PROJECT_BINARY_DIR}/mFASTTargets.cmake")
else(CMAKE_CROSSCOMPILING)
  export(TARGETS fast_type_gen ${MFAST_SHARED_LIBRARIES} ${MFAST_STATIC_LIBRARIES}
         FILE "${PROJECT_BINARY_DIR}/mFASTTargets.cmake")
endif (CMAKE_CROSSCOMPILING)

# Create the mFASTConfig.cmake for the build tree
set(CONF_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/src")
configure_file(${PROJECT_SOURCE_DIR}/cmake/mFASTConfig.cmake.in
               "${PROJECT_BINARY_DIR}/mFASTConfig.cmake"
               @ONLY)

# Create the mFASTConfig.cmake for the install tree
file(RELATIVE_PATH REL_INCLUDE_DIR "${ABSOLUTE_INSTALL_CMAKE_DIR}" "${ABSOLUTE_INSTALL_INCLUDE_DIR}")
set(CONF_INCLUDE_DIRS "\${MFAST_CMAKE_DIR}/${REL_INCLUDE_DIR}")
configure_file(${PROJECT_SOURCE_DIR}/cmake/mFASTConfig.cmake.in
               "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/mFASTConfig.cmake"
               @ONLY)

# Create the mFASTConfigVersion.cmake for both
configure_file(${PROJECT_SOURCE_DIR}/cmake/mFASTConfigVersion.cmake.in
               "${PROJECT_BINARY_DIR}/mFASTConfigVersion.cmake"
               @ONLY)

configure_file(${PROJECT_SOURCE_DIR}/cmake/FastTypeGenTarget.cmake 
               FastTypeGenTarget.cmake COPYONLY)

configure_file(${PROJECT_SOURCE_DIR}/cmake/SetCXXStandard.cmake 
               SetCXXStandard.cmake COPYONLY)
               
# Install the mFASTConfig.cmake and mFASTConfigVersion.cmake
install(FILES
  "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/mFASTConfig.cmake"
  "${PROJECT_BINARY_DIR}/mFASTConfigVersion.cmake"
  "${PROJECT_SOURCE_DIR}/cmake/FastTypeGenTarget.cmake"
  "${PROJECT_SOURCE_DIR}/cmake/SetCXXStandard.cmake"
  DESTINATION "${INSTALL_CMAKE_DIR}" COMPONENT dev)
  
# Install the export set for use with the install-tree
install(EXPORT mFASTTargets DESTINATION "${INSTALL_CMAKE_DIR}" COMPONENT dev)
