#ifndef DECIMAL_INSTRUCTION_H_Z9VE4XUK
#define DECIMAL_INSTRUCTION_H_Z9VE4XUK

#include "int_instructions.h"

namespace mfast
{
  class MFAST_EXPORT mantissa_field_instruction
    : public int64_field_instruction
  {
  public:

    mantissa_field_instruction(operator_enum_t            operator_id,
                               const op_context_t*        context,
                               int_value_storage<int64_t> initial_value)
      : int64_field_instruction(0, operator_id, presence_mandatory, 0, 0, 0, context, initial_value)
    {
    }

    mantissa_field_instruction(const mantissa_field_instruction& other)
      : int64_field_instruction(other)
    {
    }

  };


  class MFAST_EXPORT decimal_field_instruction
    : public integer_field_instruction_base
  {
  public:

    decimal_field_instruction(uint16_t              field_index,
                              operator_enum_t       decimal_operator_id,
                              presence_enum_t       optional,
                              uint32_t              id,
                              const char*           name,
                              const char*           ns,
                              const op_context_t*   decimal_context,
                              decimal_value_storage initial_value = decimal_value_storage())
      : integer_field_instruction_base(field_index,
                                       decimal_operator_id,
                                       field_type_decimal,
                                       optional,
                                       id,
                                       name,
                                       ns,
                                       decimal_context,
                                       initial_value.storage_)
      , mantissa_instruction_(0)
    {
    }

    decimal_field_instruction(uint16_t                    field_index,
                              operator_enum_t             exponent_operator_id,
                              presence_enum_t             optional,
                              uint32_t                    id,
                              const char*                 name,
                              const char*                 ns,
                              const op_context_t*         exponent_context,
                              mantissa_field_instruction* mi,
                              decimal_value_storage       initial_value = decimal_value_storage())
      : integer_field_instruction_base(field_index,
                                       exponent_operator_id,
                                       field_type_exponent,
                                       optional,
                                       id,
                                       name,
                                       ns,
                                       exponent_context,
                                       initial_value.storage_)
    {
      mantissa_instruction(mi);
    }

    decimal_field_instruction(const decimal_field_instruction& other,
                              mantissa_field_instruction*      mantissa_instruction)
      : integer_field_instruction_base(other)
      , mantissa_instruction_(mantissa_instruction)
    {
    }

    /// Perform deep copy
    virtual void copy_construct_value(const value_storage& src,
                                      value_storage&       dest,
                                      allocator*           alloc,
                                      value_storage*       fields_storage=0) const;


    virtual void construct_value(value_storage& storage,
                                 allocator*     alloc) const;


    virtual void accept(field_instruction_visitor&, void*) const;
    virtual decimal_field_instruction* clone(arena_allocator& alloc) const;

    const mantissa_field_instruction* mantissa_instruction() const
    {
      return mantissa_instruction_;
    }

    const value_storage& initial_or_default_value() const
    {
      if (initial_value_.is_empty()) {
        static const decimal_value_storage default_value(0,0);
        return default_value.storage_;
      }
      return initial_value_;
    }

    void mantissa_instruction(mantissa_field_instruction* v)
    {
      mantissa_instruction_ = v;
      this->initial_value_.of_decimal.mantissa_ = mantissa_instruction_->initial_value().get<int64_t>();

      if (has_pmap_bit_ == 0) {
        has_pmap_bit_ = mantissa_instruction_->pmap_size();
      }
    }

  protected:

    friend class dictionary_builder;
    mantissa_field_instruction* mantissa_instruction_;
  };

} /* mfast */


#endif /* end of include guard: DECIMAL_INSTRUCTION_H_Z9VE4XUK */
