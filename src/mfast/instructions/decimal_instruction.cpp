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

#include "../field_instructions.h"

namespace mfast
{
  void decimal_field_instruction::construct_value(value_storage& storage,
                                                  allocator*       ) const
  {
    storage = initial_value_;
    storage.of_decimal.present_ = !optional();
  }

  void
  decimal_field_instruction::copy_construct_value(const value_storage& src,
                                                  value_storage&       dest,
                                                  allocator* /* alloc */,
                                                  value_storage*) const
  {
    dest.of_decimal.present_ =  src.of_decimal.present_;
    dest.of_decimal.mantissa_ = src.of_decimal.mantissa_;
    dest.of_decimal.exponent_ = src.of_decimal.exponent_;
  }


  decimal_field_instruction*
  decimal_field_instruction::clone(arena_allocator& alloc) const
  {
    return new (alloc) decimal_field_instruction(*this);
  }


  const decimal_field_instruction*
  decimal_field_instruction::default_instruction()
  {
    static const decimal_field_instruction inst(operator_none,presence_mandatory,0,"","",0, decimal_value_storage());
    return &inst;
  }
} /* mfast */