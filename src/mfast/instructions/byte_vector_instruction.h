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

#ifndef BYTE_VECTOR_INSTRUCTION_H_9RXMRVCP
#define BYTE_VECTOR_INSTRUCTION_H_9RXMRVCP

#include "string_instructions.h"

namespace mfast
{
  class MFAST_EXPORT byte_vector_field_instruction
    : public unicode_field_instruction
  {
  public:
    byte_vector_field_instruction(operator_enum_t      operator_id,
                                  presence_enum_t      optional,
                                  uint32_t             id,
                                  const char*          name,
                                  const char*          ns,
                                  const op_context_t*  context,
                                  string_value_storage initial_value,
                                  uint32_t             length_id ,
                                  const char*          length_name,
                                  const char*          length_ns,
                                  instruction_tag      tag=instruction_tag())
      :  unicode_field_instruction(operator_id,
                                   optional,
                                   id, name, ns, context,
                                   initial_value,
                                   length_id,
                                   length_name,
                                   length_ns,
                                   tag,
                                   field_type_byte_vector)
    {
    }

    byte_vector_field_instruction(const byte_vector_field_instruction& other)
      : unicode_field_instruction(other)
    {
    }

    virtual void accept(field_instruction_visitor& visitor, void* context) const;
    virtual byte_vector_field_instruction* clone(arena_allocator& alloc) const;

    static std::ptrdiff_t hex2binary(const char* src, unsigned char* target);

    static const byte_vector_field_instruction* default_instruction();
  };

} /* mfast */


#endif /* end of include guard: BYTE_VECTOR_INSTRUCTION_H_9RXMRVCP */
