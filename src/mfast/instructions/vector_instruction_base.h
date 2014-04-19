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

#ifndef VECTOR_INSTRUCTION_BASE_H_C3CQUXY8
#define VECTOR_INSTRUCTION_BASE_H_C3CQUXY8

#include "field_instruction.h"

namespace mfast
{
  class MFAST_EXPORT vector_field_instruction_base
    : public field_instruction
  {
  public:

    vector_field_instruction_base(operator_enum_t   operator_id,
                                  field_type_enum_t field_type,
                                  presence_enum_t   optional,
                                  uint32_t          id,
                                  const char*       name,
                                  const char*       ns,
                                  std::size_t       element_size,
                                  instruction_tag   tag)
      : field_instruction(operator_id,
                          field_type,
                          optional,
                          id,
                          name,
                          ns,
                          tag)
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
