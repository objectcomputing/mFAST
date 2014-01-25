#ifndef VECTOR_INSTRUCTION_BASE_H_C3CQUXY8
#define VECTOR_INSTRUCTION_BASE_H_C3CQUXY8

#include "field_instruction.h"

namespace mfast
{
  class MFAST_EXPORT vector_field_instruction_base
    : public field_instruction
  {
  public:

    vector_field_instruction_base(uint16_t          field_index,
                                  operator_enum_t   operator_id,
                                  field_type_enum_t field_type,
                                  presence_enum_t   optional,
                                  uint32_t          id,
                                  const char*       name,
                                  const char*       ns,
                                  std::size_t       element_size)
      : field_instruction(field_index,
                          operator_id,
                          field_type,
                          optional,
                          id,
                          name,
                          ns)
      , element_size_(element_size)
    {
    }

    vector_field_instruction_base(const vector_field_instruction_base& other)
      : field_instruction(other)
      , element_size_(other.element_size_)
    {
    }

    virtual void construct_value(value_storage& storage,
                                 allocator*     alloc) const;
    virtual void destruct_value(value_storage& storage,
                                allocator*     alloc) const;


    // perform deep copy
    virtual void copy_construct_value(const value_storage& src,
                                      value_storage&       dest,
                                      allocator*           alloc,
                                      value_storage*       fields_storage=0) const;

  protected:
    std::size_t element_size_;
  };

} /* mfast */


#endif /* end of include guard: VECTOR_INSTRUCTION_BASE_H_C3CQUXY8 */
