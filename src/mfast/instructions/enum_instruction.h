#ifndef ENUM_INSTRUCTION_H_Y9NRM0OD
#define ENUM_INSTRUCTION_H_Y9NRM0OD

#include "int_instructions.h"

namespace mfast
{
  class MFAST_EXPORT enum_field_instruction
    : public integer_field_instruction_base
    , public referable_instruction<enum_field_instruction>
  {
  public:

    enum_field_instruction(uint16_t                      field_index,
                           operator_enum_t               operator_id,
                           presence_enum_t               optional,
                           uint32_t                      id,
                           const char*                   name,
                           const char*                   ns,
                           const op_context_t*           context,
                           int_value_storage<uint64_t>   initial_value,
                           const char**                  elements,
                           const uint64_t*               element_values,
                           uint64_t                      num_elements,
                           const enum_field_instruction* ref,
                           const char*                   cpp_ns)
      : integer_field_instruction_base(field_index,
                                       operator_id,
                                       field_type_enum,
                                       optional,
                                       id,
                                       name,
                                       ns,
                                       context,
                                       initial_value.storage_)
      , referable_instruction<enum_field_instruction>(ref, cpp_ns)
      , elements_(elements)
      , num_elements_(num_elements)
      , element_values_(element_values)
    {
    }

    enum_field_instruction(const enum_field_instruction &other)
      : integer_field_instruction_base(other)
      , referable_instruction<enum_field_instruction>(other)
      , elements_(other.elements_)
      , num_elements_(other.num_elements_)
      , element_values_(other.element_values_)
    {
    }

    virtual void accept(field_instruction_visitor& visitor, void* context) const;
    virtual enum_field_instruction* clone(arena_allocator& alloc) const;

    const char* element_name(uint64_t v) const
    {
      if (element_values_ == 0)
        return elements_[v];
      const uint64_t* it = std::lower_bound(element_values_, element_values_+num_elements_, v);
      if (it != element_values_+num_elements_ && *it == v)
        return elements_[it-element_values_];
      return 0;
    }

    uint64_t num_elements() const
    {
      return num_elements_;
    }

    const char** elements() const
    {
      return elements_;
    }

    const uint64_t* element_values() const
    {
      return element_values_;
    }

    const char** elements_;
    uint64_t num_elements_;
    const uint64_t* element_values_;
  };

} /* mfast */


#endif /* end of include guard: ENUM_INSTRUCTION_H_Y9NRM0OD */
