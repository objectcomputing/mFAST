# add_subdirectory (fastxml)
add_subdirectory (mfast)

if (BUILD_FAST_TYPE_GEN)
    add_subdirectory (fast_type_gen)
endif()

install(FILES "mfast.h" DESTINATION "${INSTALL_INCLUDE_DIR}")
