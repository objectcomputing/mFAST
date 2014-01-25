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

#include "int_instructions.h"

namespace mfast
{
  void integer_field_instruction_base::construct_value(value_storage& storage,
                                                       allocator* /* alloc */) const
  {
    storage.of_uint.content_ = initial_value_.of_uint.content_;
    storage.of_uint.defined_bit_ = 1;
    storage.of_uint.present_ = !optional();
  }

  const value_storage integer_field_instruction_base::default_value_(1);

} /* mfast */