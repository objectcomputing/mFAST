# add_subdirectory (fastxml)
add_subdirectory (mfast)

add_subdirectory (fast_type_gen)
set(FAST_TYPE_GEN_INSTALL_LOCATION "${FAST_TYPE_GEN_INSTALL_LOCATION}" PARENT_SCOPE)

install(FILES "mfast.h" DESTINATION "${INSTALL_INCLUDE_DIR}")
