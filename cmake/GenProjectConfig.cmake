# Provide mfast-config.cmake and mfast-config.version to be used by other applications
# ===============================

include(CMakePackageConfigHelpers)

# Create the mFASTConfig.cmake for the install tree

write_basic_package_version_file(${PROJECT_BINARY_DIR}/mFASTConfigVersion.cmake
                                 VERSION ${mFAST_VERSION}
                                 COMPATIBILITY SameMajorVersion)

configure_package_config_file(${PROJECT_SOURCE_DIR}/cmake/mFASTConfig.cmake.in
                              ${PROJECT_BINARY_DIR}/mFASTConfig.cmake
                              INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/mFAST)

install(FILES ${PROJECT_BINARY_DIR}/mFASTConfigVersion.cmake
              ${PROJECT_BINARY_DIR}/mFASTConfig.cmake
              ${PROJECT_SOURCE_DIR}/cmake/FastTypeGenTarget.cmake 
              DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/mFAST)

install(EXPORT mFASTTargets 
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/mFAST
        COMPONENT dev)
