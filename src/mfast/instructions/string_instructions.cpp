#include "string_instructions.h"

namespace mfast
{
  void ascii_field_instruction::construct_value(value_storage& storage,
                                                allocator*       ) const
  {
    storage = initial_value_;
    if (optional())
      storage.of_array.len_ = 0;
    storage.of_array.defined_bit_ = 1;
  }

  void ascii_field_instruction::copy_construct_value(const value_storage& src,
                                                     value_storage&       dest,
                                                     allocator*           alloc,
                                                     value_storage*) const
  {
    dest.of_array.defined_bit_ = 1;
    size_t len = src.of_array.len_;
    if (len && src.of_array.content_ != initial_value_.of_array.content_) {
      dest.of_array.content_ = 0;
      dest.of_array.capacity_in_bytes_ = alloc->reallocate(dest.of_array.content_, 0, len * element_size_);
      std::memcpy(dest.of_array.content_, src.of_array.content_, len * element_size_);
    }
    else {
      dest.of_array.content_ = src.of_array.content_;
      dest.of_array.capacity_in_bytes_ = 0;
    }
    dest.of_array.len_ = len;
  }


  ascii_field_instruction*
  ascii_field_instruction::clone(arena_allocator& alloc) const
  {
    return new (alloc) ascii_field_instruction(*this);
  }

  const value_storage ascii_field_instruction::default_value_("");

  unicode_field_instruction*
  unicode_field_instruction::clone(arena_allocator& alloc) const
  {
    return new (alloc) unicode_field_instruction(*this);
  }

} /* mfast */