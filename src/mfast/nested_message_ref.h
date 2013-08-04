// Copyright (c) 2013, Huang-Ming Huang,  Object Computing, Inc.
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
#ifndef DYNAMIC_MESSAGE_REF_H_X549MWYP
#define DYNAMIC_MESSAGE_REF_H_X549MWYP

#include "mfast/field_instruction.h"
#include "mfast/field_ref.h"
#include "mfast/exceptions.h"
#include "mfast/message_ref.h"

namespace mfast
{

class nested_message_cref
  : public field_cref
{
  public:
    typedef const templateref_instruction* instruction_cptr;
    typedef boost::false_type is_mutable;

    nested_message_cref(const value_storage* storage,
                        instruction_cptr     inst)
      : field_cref(storage, inst)
    {
    }

    explicit nested_message_cref(const field_cref& other)
      : field_cref(other)
    {
    }

    const templateref_instruction* instruction() const
    {
      return static_cast<const templateref_instruction*> (instruction_);
    }

    bool is_static() const
    {
      return instruction()->name() != 0;
    }
    
    message_cref target() const 
    {
      return message_cref(*this);
    }

};


class nested_message_mref
  : public make_field_mref<nested_message_cref>
{
  public:
    typedef boost::true_type is_mutable;
    typedef mfast::allocator allocator_type;

    nested_message_mref(allocator_type*                alloc,
                        value_storage*                 storage,
                        const templateref_instruction* inst)
      : make_field_mref<nested_message_cref>(alloc, storage, inst)
    {
    }

    nested_message_mref(const field_mref_base& other)
      : make_field_mref<nested_message_cref>(other)
    {
    }
    
    message_mref target() const 
    {
      return message_mref(alloc_, storage(), storage()->of_templateref.of_instruction.instruction_);
    }

    template <typename MESSAGE_MREF>
    MESSAGE_MREF as() const
    {
      set_nested_instruction(MESSAGE_MREF::the_instruction, true);
      return MESSAGE_MREF(alloc_, storage());
    }

    message_mref rebind(const template_instruction* inst) const
    {
      set_nested_instruction(inst, true);
      return message_mref(alloc_, storage(), inst);
    }

    void set_nested_instruction(const template_instruction* inst, bool construct_subfields = true) const
    {
      const template_instruction*& target = storage()->of_templateref.of_instruction.instruction_;
      
      assert(!is_static() || target == 0);

      if (inst == target)
        return;

      if (target) {
        target->destruct_value(*storage(), alloc_);
      }
      inst->construct_value(*storage(), 0, alloc_, construct_subfields);
      target = inst;
    }
};


}


#endif /* end of include guard: DYNAMIC_MESSAGE_REF_H_X549MWYP */
