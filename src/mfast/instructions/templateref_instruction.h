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

#ifndef TEMPLATEREF_INSTRUCTION_H_54TZ9IPX
#define TEMPLATEREF_INSTRUCTION_H_54TZ9IPX

#include "template_instruction.h"

namespace mfast
{
  class MFAST_EXPORT templateref_instruction
    : public field_instruction
  {
  public:


    templateref_instruction(instruction_tag tag = instruction_tag());

    virtual void construct_value(value_storage& storage,
                                 allocator*     alloc) const;
    virtual void destruct_value(value_storage& storage,
                                allocator*     alloc) const;

    void  construct_value(value_storage&              storage,
                          allocator*                  alloc,
                          const template_instruction* from_inst,
                          bool                        construct_subfields) const;

    virtual std::size_t pmap_size() const;

    /// Perform deep copy
    virtual void copy_construct_value(const value_storage& src,
                                      value_storage&       dest,
                                      allocator*           alloc,
                                      value_storage*       fields_storage=0) const;

    virtual void accept(field_instruction_visitor&, void*) const;

    virtual templateref_instruction* clone(arena_allocator& alloc) const;

    static instructions_view_t default_instruction();
  };

} /* mfast */

#endif /* end of include guard: TEMPLATEREF_INSTRUCTION_H_54TZ9IPX */
