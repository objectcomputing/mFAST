#ifndef FIELD_MREF_AS_H_ZXBC0LR9
#define FIELD_MREF_AS_H_ZXBC0LR9


#include "mfast/int_ref.h"
#include "mfast/decimal_ref.h"
#include "mfast/string_ref.h"

namespace mfast
{
class field_mref
  : public make_field_mref<field_cref>
{
  public:
    field_mref()
    {
    }

    field_mref(allocator*       alloc,
               value_storage*   storage,
               instruction_cptr instruction)
      : make_field_mref<field_cref>(alloc, storage, instruction)
    {
    }

    field_mref(const make_field_mref<field_cref> &other)
      : make_field_mref<field_cref>(other)
    {
    }


    template <typename T>
    void as(T t);
    
    void as(const char*);
    
    void as(const decimal&);
    
    template <int SIZE>
    void as(unsigned char (&value)[SIZE]) ;
    
    void as(const std::vector<unsigned char>& value) ;
};

template <>
struct mref_of<field_cref>
{
  typedef field_mref type;
};

template <typename T>
void field_mref::as(T value)
{
  switch (this->instruction()->field_type())
  {
  case field_type_int32:
    this->static_cast_as<int32_mref>().as(value);
    break;
  case field_type_uint32:
    this->static_cast_as<uint32_mref>().as(value);
    break;
  case field_type_int64:
    this->static_cast_as<int64_mref>().as(value);
    break;
  case field_type_uint64:
    this->static_cast_as<uint64_mref>().as(value);
    break;
  default:
    throw std::bad_cast();
  }
}

inline void field_mref::as(const char* value)
{
  switch (this->instruction()->field_type())
  {
  case field_type_ascii_string:
    this->static_cast_as<ascii_string_mref>().as(value);
    break;
  case field_type_unicode_string:
    this->static_cast_as<unicode_string_mref>().as(value);
    break;
  default:
    throw std::bad_cast();
  }  
}

inline void field_mref::as(const decimal& value)
{
  if (this->instruction()->field_type() == field_type_decimal) 
    this->static_cast_as<decimal_mref>().as(value);
  else
    throw std::bad_cast();
}

template <int SIZE>  
void field_mref::as(unsigned char (&value)[SIZE])
{
  if (this->instruction()->field_type() == field_type_byte_vector) 
    this->static_cast_as<byte_vector_mref>().as(value);
  else 
    throw std::bad_cast();    
}

inline void field_mref::as(const std::vector<unsigned char>& value)
{
  if (this->instruction()->field_type() == field_type_byte_vector) 
    this->static_cast_as<byte_vector_mref>().as(value);
  else 
    throw std::bad_cast();
}

namespace detail {

inline field_mref
field_ref_with_id(value_storage*              storage,
                  const group_content_helper* helper,
                  allocator*                  alloc,
                  uint32_t                    id)
{
  if (helper) {
    int index = helper->find_subinstruction_index_by_id(id);
    if (index >= 0)
      return field_mref(alloc, &storage[index], helper->subinstructions_[index]);
  }
  return field_mref();
}

inline field_mref
field_ref_with_name(value_storage*              storage,
                    const group_content_helper* helper,
                    allocator*                  alloc,
                    const char*                 name)
{
  if (helper) {
    int index = helper->find_subinstruction_index_by_name(name);
    if (index >= 0)
      return field_mref(alloc, &storage[index], helper->subinstructions_[index]);
  }
  return field_mref();
}

}

}

#endif /* end of include guard: FIELD_MREF_AS_H_ZXBC0LR9 */
