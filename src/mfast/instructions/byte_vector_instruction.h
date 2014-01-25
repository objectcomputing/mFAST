#ifndef BYTE_VECTOR_INSTRUCTION_H_9RXMRVCP
#define BYTE_VECTOR_INSTRUCTION_H_9RXMRVCP

#include "string_instructions.h"

namespace mfast
{
  class MFAST_EXPORT byte_vector_field_instruction
    : public unicode_field_instruction
  {
  public:
    byte_vector_field_instruction(uint16_t             field_index,
                                  operator_enum_t      operator_id,
                                  presence_enum_t      optional,
                                  uint32_t             id,
                                  const char*          name,
                                  const char*          ns,
                                  const op_context_t*  context,
                                  string_value_storage initial_value = string_value_storage(),
                                  uint32_t             length_id = 0,
                                  const char*          length_name = "",
                                  const char*          length_ns = "")
      :  unicode_field_instruction(field_index,
                                   operator_id,
                                   optional,
                                   id, name, ns, context,
                                   initial_value,
                                   length_id,
                                   length_name,
                                   length_ns,
                                   field_type_byte_vector)
    {
    }

    byte_vector_field_instruction(const byte_vector_field_instruction& other)
      : unicode_field_instruction(other)
    {
    }

    virtual void accept(field_instruction_visitor& visitor, void* context) const;
    virtual byte_vector_field_instruction* clone(arena_allocator& alloc) const;
  };

} /* mfast */


#endif /* end of include guard: BYTE_VECTOR_INSTRUCTION_H_9RXMRVCP */
