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
#ifndef NESTED_MESSAGE_REF_H_X549MWYP
#define NESTED_MESSAGE_REF_H_X549MWYP

#include "mfast/field_instructions.h"
#include "mfast/field_ref.h"
#include "mfast/message_ref.h"

namespace mfast
{

  class nested_message_cref
    : public field_cref
  {
  public:
    typedef templateref_instruction instruction_type;
    typedef const templateref_instruction* instruction_cptr;
    typedef boost::false_type is_mutable;

    nested_message_cref()
    {
    }

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

    message_cref target() const
    {
      return message_cref(this->storage()->of_templateref.content_, this->target_instruction());
    }

    const template_instruction* target_instruction() const
    {
      return storage()->of_templateref.of_instruction.instruction_;
    }

    operator aggregate_cref() const
    {
      return aggregate_cref(this->storage()->of_templateref.content_, this->target_instruction());
    }

    template <typename FieldAccessor>
    void accept_accessor(FieldAccessor&) const;
  };


  class nested_message_mref
    : public make_field_mref<nested_message_cref>
  {
  public:
    typedef boost::true_type is_mutable;
    typedef mfast::allocator allocator_type;

    nested_message_mref()
    {
    }

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
      return message_mref(alloc_, this->storage()->of_templateref.content_, this->target_instruction());
    }

    operator aggregate_mref() const
    {
      return aggregate_mref(alloc_, storage()->of_templateref.content_, this->target_instruction());
    }

    template <typename FieldMutator>
    void accept_mutator(FieldMutator&) const;

    template <typename MESSAGE>
    typename MESSAGE::mref_type as() const
    {
      typedef typename MESSAGE::mref_type mref_type;
      set_target_instruction(MESSAGE::instruction(), true);
      return mref_type(alloc_, this->storage()->of_templateref.content_, MESSAGE::instruction());
    }

    message_mref rebind(const template_instruction* inst) const
    {
      set_target_instruction(inst, true);
      return message_mref(alloc_, this->storage()->of_templateref.content_, inst);
    }

    void set_target_instruction(const template_instruction* inst, bool construct_subfields = true) const
    {
      const templateref_instruction* templateRef_inst = static_cast<const templateref_instruction*>(this->instruction_);
      const template_instruction*& target_inst = this->storage()->of_templateref.of_instruction.instruction_;

      // assert(target_inst == 0);

      if (inst == target_inst)
        return;

      if (target_inst) {
        templateRef_inst->destruct_value(*this->storage(), alloc_);
      }
      templateRef_inst->construct_value(*this->storage(), alloc_, inst, construct_subfields);

    }

  };


}


#endif /* end of include guard: NESTED_MESSAGE_REF_H_X549MWYP */
