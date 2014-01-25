#include "templateref_instruction.h"

namespace mfast
{
  void templateref_instruction::construct_value(value_storage& storage,
                                                allocator*     alloc) const
  {
    this->construct_value(storage, alloc, target_, true);
  }

  void templateref_instruction::construct_value(value_storage&              storage,
                                                allocator*                  alloc,
                                                const template_instruction* from_inst,
                                                bool                        construct_subfields) const
  {
    storage.of_templateref.of_instruction.instruction_ = from_inst;
    if (from_inst) {
      storage.of_templateref.content_ = static_cast<value_storage*>(
        alloc->allocate(from_inst->group_content_byte_count()));

      if (construct_subfields)
        from_inst->construct_group_subfields(storage.of_templateref.content_, alloc);
      else
        memset(storage.of_templateref.content_, 0, from_inst->group_content_byte_count());
    }
    else {
      storage.of_templateref.content_ = 0;
    }
  }

  void templateref_instruction::destruct_value(value_storage& storage,
                                               allocator*     alloc) const
  {
    if (storage.of_templateref.of_instruction.instruction_ && storage.of_templateref.content_) {
      storage.of_templateref.of_instruction.instruction_->destruct_group_subfields(
        static_cast<value_storage*>(storage.of_templateref.content_),
        alloc);
      alloc->deallocate(storage.of_templateref.content_, storage.of_templateref.of_instruction.instruction_->group_content_byte_count());
    }
  }

  void templateref_instruction::copy_construct_value(const value_storage& src,
                                                     value_storage&       dest,
                                                     allocator*           alloc,
                                                     value_storage*) const
  {
    dest.of_templateref.of_instruction.instruction_ = src.of_templateref.of_instruction.instruction_;
    if (src.of_templateref.of_instruction.instruction_) {
      dest.of_templateref.content_ =
        static_cast<value_storage*>(alloc->allocate( dest.of_templateref.of_instruction.instruction_->group_content_byte_count() ));

      dest.of_templateref.of_instruction.instruction_->copy_group_subfields(
        src.of_templateref.content_,
        dest.of_templateref.content_,
        alloc);
    }
  }

  templateref_instruction*
  templateref_instruction::clone(arena_allocator& alloc) const
  {
    return new (alloc) templateref_instruction(*this);
  }

  std::size_t templateref_instruction::pmap_size() const
  {
    if (target_) {
      return target_->segment_pmap_size();
    }
    return 0;
  }

  const const_instruction_ptr_t*
  templateref_instruction::default_instruction()
  {
    static const templateref_instruction the_instruction(0);
    static const field_instruction* array[] = {
      &the_instruction,
    };
    return array;
  }

} /* mfast */
