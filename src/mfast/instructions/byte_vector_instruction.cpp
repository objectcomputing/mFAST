#include "byte_vector_instruction.h"

namespace mfast
{

  byte_vector_field_instruction*
  byte_vector_field_instruction::clone(arena_allocator& alloc) const
  {
    return new (alloc) byte_vector_field_instruction(*this);
  }

} /* mfast */
