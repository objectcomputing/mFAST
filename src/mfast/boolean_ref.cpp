#include "boolean_ref.h"

namespace mfast
{

const boolean::instruction_type*
boolean::instruction()
{
  static const char* boolean_elements[] = {"false","true"};

  static const boolean_cref::instruction_type
    the_instruction(0,operator_none,presence_mandatory,0,"boolean","",0,0,boolean_elements,2,0,"mfast");

  return &the_instruction;
}

} /* mfast */
