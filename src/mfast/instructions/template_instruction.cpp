#include "template_instruction.h"

namespace mfast
{
  void template_instruction::copy_construct_value(const value_storage& src,
                                                  value_storage&       dest,
                                                  allocator*           alloc,
                                                  value_storage*       dest_fields_storage) const
  {
    if (dest_fields_storage) {
      dest.of_group.own_content_ = false;
    }
    else {
      dest.of_group.own_content_ = true;
      dest_fields_storage = static_cast<value_storage*>(
        alloc->allocate(this->group_content_byte_count()));
    }
    dest.of_group.content_ = dest_fields_storage;
    copy_group_subfields(src.of_group.content_,
                         dest_fields_storage,
                         alloc);

  }

  template_instruction*
  template_instruction::clone(arena_allocator& alloc) const
  {
    return new (alloc) template_instruction(*this);
  }

} /* mfast */