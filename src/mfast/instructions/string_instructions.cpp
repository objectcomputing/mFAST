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

#include "string_instructions.h"

namespace mfast
{

  const value_storage ascii_field_instruction::default_value_("");


  ascii_field_instruction::ascii_field_instruction(operator_enum_t      operator_id,
                                                   presence_enum_t      optional,
                                                   uint32_t             id,
                                                   const char*          name,
                                                   const char*          ns,
                                                   const op_context_t*  context,
                                                   string_value_storage initial_value,
                                                   instruction_tag      tag,
                                                   field_type_enum_t    field_type)
    :  vector_field_instruction_base(operator_id,
                                     field_type,
                                     optional,
                                     id, name, ns,
                                     sizeof(char),
                                     tag)
    , op_context_(context)
    , initial_value_(initial_value.storage_)
    , prev_value_(&prev_storage_)
    , initial_or_default_value_(initial_value_.is_empty() ? &default_value_ : &initial_value_)
  {
    mandatory_no_initial_value_ = !optional && initial_value.storage_.is_empty();
  }

  ascii_field_instruction::ascii_field_instruction(const ascii_field_instruction& other)
    : vector_field_instruction_base(other)
    , op_context_(other.op_context_)
    , initial_value_(other.initial_value_)
    , prev_value_(&prev_storage_)
    , initial_or_default_value_(initial_value_.is_empty() ? &default_value_ : &initial_value_)
  {
  }

  void ascii_field_instruction::construct_value(value_storage& storage,
                                                allocator*       ) const
  {
    storage = initial_value_;
    if (optional())
      storage.of_array.len_ = 0;
    storage.of_array.defined_bit_ = 1;
  }

  void ascii_field_instruction::copy_construct_value(const value_storage& src,
                                                     value_storage&       dest,
                                                     allocator*           alloc,
                                                     value_storage*) const
  {
    dest.of_array.defined_bit_ = 1;
    size_t len = src.of_array.len_;
    if (len && src.of_array.content_ != initial_value_.of_array.content_) {
      dest.of_array.content_ = 0;
      dest.of_array.capacity_in_bytes_ = alloc->reallocate(dest.of_array.content_, 0, len * element_size_);
      std::memcpy(dest.of_array.content_, src.of_array.content_, len * element_size_);
    }
    else {
      dest.of_array.content_ = src.of_array.content_;
      dest.of_array.capacity_in_bytes_ = 0;
    }
    dest.of_array.len_ = static_cast<uint32_t>(len);
  }

  ascii_field_instruction*
  ascii_field_instruction::clone(arena_allocator& alloc) const
  {
    return new (alloc) ascii_field_instruction(*this);
  }

  const ascii_field_instruction* ascii_field_instruction::default_instruction()
  {
    static const ascii_field_instruction inst(operator_none,presence_mandatory,0,"","",0, string_value_storage());
    return &inst;
  }

  void ascii_field_instruction::initial_value(const value_storage& v)
  {
    initial_value_ = v;
    initial_or_default_value_ = initial_value_.is_empty() ? &default_value_ : &initial_value_;
    mandatory_no_initial_value_ = !optional() && initial_value_.is_empty();
  }

  unicode_field_instruction*
  unicode_field_instruction::clone(arena_allocator& alloc) const
  {
    return new (alloc) unicode_field_instruction(*this);
  }

  const unicode_field_instruction* unicode_field_instruction::default_instruction()
  {
    static const unicode_field_instruction inst(operator_none,presence_mandatory,0,"","",0, string_value_storage(), 0, "", "");
    return &inst;
  }



} /* mfast */
