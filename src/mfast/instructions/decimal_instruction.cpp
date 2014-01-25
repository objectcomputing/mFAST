#include "decimal_instruction.h"

namespace mfast
{
  void decimal_field_instruction::construct_value(value_storage& storage,
                                                  allocator*       ) const
  {
    storage = initial_value_;
    storage.of_decimal.present_ = !optional();
  }

  void
  decimal_field_instruction::copy_construct_value(const value_storage& src,
                                                  value_storage&       dest,
                                                  allocator* /* alloc */,
                                                  value_storage*) const
  {
    dest.of_decimal.present_ =  src.of_decimal.present_;
    dest.of_decimal.mantissa_ = src.of_decimal.mantissa_;
    dest.of_decimal.exponent_ = src.of_decimal.exponent_;
  }


  decimal_field_instruction*
  decimal_field_instruction::clone(arena_allocator& alloc) const
  {
    return new (alloc) decimal_field_instruction(*this);
  }
} /* mfast */