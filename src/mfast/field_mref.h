// Copyright (c) 2013, 2014, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
//
//     mFAST is free software: you can redistribute it and/or modify
//     it under the terms of the GNU Lesser General Public License as published by
//     the Free Software Foundation, either version 3 of the License, or
//     (at your option) any later version.
//
//     mFAST is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.
//
//     You should have received a copy of the GNU Lesser General Public License
//     along with mFast.  If not, see <http://www.gnu.org/licenses/>.
//

#ifndef FIELD_MREF_AS_H_ZXBC0LR9
#define FIELD_MREF_AS_H_ZXBC0LR9


#include "mfast/int_ref.h"
#include "mfast/decimal_ref.h"
#include "mfast/string_ref.h"
#include <boost/exception/all.hpp>
namespace mfast
{

  struct tag_input_type;
  struct tag_output_type;
  typedef boost::error_info<tag_input_type,std::string> input_type_info;
  typedef boost::error_info<tag_output_type,std::string> output_type_info;

  class incompatible_type_conversion_error
    : public virtual boost::exception, public virtual std::exception
  {
  public:
    incompatible_type_conversion_error(const char* input_type_name,
                                       const char* output_type_name)
    {
      *this << input_type_info(input_type_name)
            << output_type_info(output_type_name);
    }

  };

  class field_mref
    : public field_mref_base
  {
  public:
    field_mref()
    {
    }

    field_mref(allocator_type*  alloc,
               value_storage*   storage,
               instruction_cptr instruction)
      : field_mref_base(alloc, storage, instruction)
    {
    }

    field_mref(const make_field_mref<field_cref> &other)
      : field_mref_base(other)
    {
    }

    template <typename T>
    void as(T t);

    void as(const char*);

    void as(const decimal&);

    template <int SIZE>
    void as(unsigned char (&value)[SIZE]);

    void as(const std::vector<unsigned char>& value);


    template <typename FieldMutator>
    void accept_mutator(FieldMutator&) const;
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
      static_cast<int32_mref>(*this).as(static_cast<int32_t>(value));
      break;
    case field_type_uint32:
      static_cast<uint32_mref>(*this).as(static_cast<uint32_t>(value));
      break;
    case field_type_int64:
      static_cast<int64_mref>(*this).as(static_cast<int64_t>(value));
      break;
    case field_type_uint64:
    case field_type_enum:
      static_cast<uint64_mref>(*this).as(static_cast<uint64_t>(value));
      break;
    case field_type_decimal:
      static_cast<decimal_mref>(*this).as(value);
      break;
    default:
      BOOST_THROW_EXCEPTION(incompatible_type_conversion_error(typeid(value).name(), this->instruction()->field_type_name()));
    }
  }

  inline void field_mref::as(const char* value)
  {
    switch (this->instruction()->field_type())
    {
    case field_type_ascii_string:
      static_cast<ascii_string_mref>(*this).as(value);
      break;
    case field_type_unicode_string:
      static_cast<unicode_string_mref>(*this).as(value);
      break;
    default:
      BOOST_THROW_EXCEPTION(incompatible_type_conversion_error("const char*", this->instruction()->field_type_name()));
    }
  }

  inline void field_mref::as(const decimal& value)
  {
    if (this->instruction()->field_type() == field_type_decimal)
      static_cast<decimal_mref>(*this).as(value);
    else
      BOOST_THROW_EXCEPTION(incompatible_type_conversion_error("decimal", this->instruction()->field_type_name()));
  }

  template <int SIZE>
  void field_mref::as(unsigned char (&value)[SIZE])
  {
    if (this->instruction()->field_type() == field_type_byte_vector)
      static_cast<byte_vector_mref>(*this).as(value);
    else
      BOOST_THROW_EXCEPTION(incompatible_type_conversion_error("unsigned char array", this->instruction()->field_type_name()));
  }

  inline void field_mref::as(const std::vector<unsigned char>& value)
  {
    if (this->instruction()->field_type() == field_type_byte_vector)
      static_cast<byte_vector_mref>(*this).as(value);
    else
      BOOST_THROW_EXCEPTION(incompatible_type_conversion_error("std::vector<unsigned char>", this->instruction()->field_type_name()));
  }

  namespace detail {

    inline field_mref
    field_ref_with_id(value_storage*                 storage,
                      const group_field_instruction* helper,
                      allocator*                     alloc,
                      uint32_t                       id)
    {
      if (helper) {
        int index = helper->find_subinstruction_index_by_id(id);
        if (index >= 0)
          return field_mref(alloc, &storage[index], helper->subinstruction(index));
      }
      return field_mref();
    }

    inline field_mref
    field_ref_with_name(value_storage*                 storage,
                        const group_field_instruction* helper,
                        allocator*                     alloc,
                        const char*                    name)
    {
      if (helper) {
        int index = helper->find_subinstruction_index_by_name(name);
        if (index >= 0)
          return field_mref(alloc, &storage[index], helper->subinstruction(index));
      }
      return field_mref();
    }

  }

}

#endif /* end of include guard: FIELD_MREF_AS_H_ZXBC0LR9 */
