#include "int_instructions.h"

namespace mfast
{
  void integer_field_instruction_base::construct_value(value_storage& storage,
                                                       allocator* /* alloc */) const
  {
    storage.of_uint.content_ = initial_value_.of_uint.content_;
    storage.of_uint.defined_bit_ = 1;
    storage.of_uint.present_ = !optional();
  }

  const value_storage integer_field_instruction_base::default_value_(1);

} /* mfast */