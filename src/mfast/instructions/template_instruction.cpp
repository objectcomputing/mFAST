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

#include "template_instruction.h"

namespace mfast
{
  void template_instruction::copy_construct_value(const value_storage& src,
                                                  value_storage&       dest,
                                                  allocator*           alloc,
                                                  value_storage*       dest_fields_storage) const
  {
    if (dest_fields_storage) {
      dest.of_group.own_content_ = false;
    }
    else {
      dest.of_group.own_content_ = true;
      dest_fields_storage = static_cast<value_storage*>(
        alloc->allocate(this->group_content_byte_count()));
    }
    dest.of_group.content_ = dest_fields_storage;
    copy_group_subfields(src.of_group.content_,
                         dest_fields_storage,
                         alloc);

  }

  template_instruction*
  template_instruction::clone(arena_allocator& alloc) const
  {
    return new (alloc) template_instruction(*this);
  }

} /* mfast */