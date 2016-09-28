#============================================================
# FASTTYPEGEN_TARGET (public function)
#============================================================
#

function (FASTTYPEGEN_TARGET Name)
  set(FASTTYPEGEN_TARGET_usage "FASTTYPEGEN_TARGET(<Name> Input1 Input2 ...]" PARENT_SCOPE)

  set("FASTTYPEGEN_${Name}_OUTPUTS")
  set(INPUTS)

  foreach (input IN LISTS ARGN)
    get_filename_component(noext_name "${input}" NAME_WE)
    list(APPEND "FASTTYPEGEN_${Name}_OUTPUTS"
         "${CMAKE_CURRENT_BINARY_DIR}/${noext_name}.cpp"
         "${CMAKE_CURRENT_BINARY_DIR}/${noext_name}.h"
         "${CMAKE_CURRENT_BINARY_DIR}/${noext_name}.inl")
    list(APPEND INPUTS "${CMAKE_CURRENT_SOURCE_DIR}/${input}")
  endforeach (input)

  add_custom_command(
      OUTPUT ${FASTTYPEGEN_${Name}_OUTPUTS}
      COMMAND $<TARGET_FILE:fast_type_gen> -- ${INPUTS}
      DEPENDS ${ARGN} fast_type_gen
      COMMENT "[FASTTYPEGEN][${Name}] Building Fast Application Types"
      WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
      VERBATIM)

  include_directories(BEFORE "${CMAKE_CURRENT_BINARY_DIR}")

  set("FASTTYPEGEN_${Name}_OUTPUTS" "${FASTTYPEGEN_${Name}_OUTPUTS}" PARENT_SCOPE)
  set("FASTTYPEGEN_${Name}_DEFINED" TRUE PARENT_SCOPE)
  set("FASTTYPEGEN_${Name}_INPUTS" "${ARGN}" PARENT_SCOPE)
endfunction (FASTTYPEGEN_TARGET)