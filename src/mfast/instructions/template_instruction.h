#ifndef TEMPLATE_INSTRUCTION_H_PARP21ZA
#define TEMPLATE_INSTRUCTION_H_PARP21ZA

#include "group_instruction.h"

namespace mfast
{
  class MFAST_EXPORT template_instruction
    : public group_field_instruction
  {
  public:
    template_instruction(uint32_t                       id,
                         const char*                    name,
                         const char*                    ns,
                         const char*                    template_ns,
                         const char*                    dictionary,
                         const const_instruction_ptr_t* subinstructions,
                         uint32_t                       subinstructions_count,
                         bool                           reset,
                         const char*                    typeref_name="",
                         const char*                    typeref_ns="",
                         const char*                    cpp_ns="")
      : group_field_instruction(0, presence_mandatory,
                                id,
                                name,
                                ns,
                                dictionary,
                                subinstructions,
                                subinstructions_count,
                                typeref_name,
                                typeref_ns,
                                cpp_ns)
      , template_ns_(template_ns)
      , reset_(reset)
    {
      field_type_ = field_type_template;
    }

    const char* template_ns() const
    {
      return template_ns_;
    }

    void copy_construct_value(value_storage&       storage,
                              value_storage*       fields_storage,
                              allocator*           alloc,
                              const value_storage* src_fields_storage) const;

    virtual void copy_construct_value(const value_storage& src,
                                      value_storage&       dest,
                                      allocator*           alloc,
                                      value_storage*       fields_storage=0) const;


    virtual void accept(field_instruction_visitor&, void*) const;
    virtual template_instruction* clone(arena_allocator& alloc) const;

    bool has_reset_attribute() const
    {
      return reset_;
    }

    // void ensure_valid_storage(value_storage& storage,
    //                           allocator*     alloc) const;

  private:
    const char* template_ns_;
    bool reset_;
  };

  template <typename T>
  class template_instruction_ex
    : public template_instruction
  {
  public:
    template_instruction_ex(uint32_t                       id,
                            const char*                    name,
                            const char*                    ns,
                            const char*                    template_ns,
                            const char*                    dictionary,
                            const const_instruction_ptr_t* subinstructions,
                            uint32_t                       subinstructions_count,
                            bool                           reset,
                            const char*                    typeref_name="",
                            const char*                    typeref_ns="")
      : template_instruction(id, name, ns, template_ns, dictionary,
                             subinstructions, subinstructions_count, reset, typeref_name, typeref_ns)
    {
    }

    virtual template_instruction_ex<T>* clone(arena_allocator& alloc) const
    {
      return new (alloc) template_instruction_ex<T>(*this);
    }

  };
} /* mfast */


#endif /* end of include guard: TEMPLATE_INSTRUCTION_H_PARP21ZA */
