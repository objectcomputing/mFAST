#include "enum_instruction.h"

namespace mfast
{

  enum_field_instruction*
  enum_field_instruction::clone(arena_allocator& alloc) const
  {
    return new (alloc) enum_field_instruction(*this);
  }

} /* mfast */