#ifndef TEMPLATEREF_INSTRUCTION_H_54TZ9IPX
#define TEMPLATEREF_INSTRUCTION_H_54TZ9IPX

#include "template_instruction.h"

namespace mfast
{
  class MFAST_EXPORT templateref_instruction
    : public field_instruction
  {
  public:


    templateref_instruction(uint16_t                    field_index,
                            const template_instruction* ref =0)
      : field_instruction(field_index, operator_none,
                          field_type_templateref,
                          presence_mandatory,
                          0,
                          ref ? ref->name() : "",
                          ref ? ref->ns() : "")
      , target_(ref)
    {
    }

    virtual void construct_value(value_storage& storage,
                                 allocator*     alloc) const;
    virtual void destruct_value(value_storage& storage,
                                allocator*     alloc) const;

    void  construct_value(value_storage&              storage,
                          allocator*                  alloc,
                          const template_instruction* from_inst,
                          bool                        construct_subfields) const;

    virtual std::size_t pmap_size() const;

    /// Perform deep copy
    virtual void copy_construct_value(const value_storage& src,
                                      value_storage&       dest,
                                      allocator*           alloc,
                                      value_storage*       fields_storage=0) const;

    virtual void accept(field_instruction_visitor&, void*) const;

    const template_instruction* target() const
    {
      return target_;
    }

    virtual templateref_instruction* clone(arena_allocator& alloc) const;

    static const const_instruction_ptr_t* default_instruction();

  private:
    const template_instruction* target_;
  };

} /* mfast */

#endif /* end of include guard: TEMPLATEREF_INSTRUCTION_H_54TZ9IPX */
