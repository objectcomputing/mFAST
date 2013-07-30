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
#ifndef DYNAMIC_REF_H_X549MWYP
#define DYNAMIC_REF_H_X549MWYP

#include "mfast/field_instruction.h"
#include "mfast/field_ref.h"
#include "mfast/exceptions.h"
#include "mfast/message_ref.h"

namespace mfast
{

class null_dynamic_ref
  : public virtual boost::exception, public virtual std::exception
{
  public:
    null_dynamic_ref()
    {
    }

};

struct encoder_impl;
class dynamic_cref
  : public field_cref
{
  public:
    typedef const templateref_instruction* instruction_cptr;
    typedef boost::false_type is_mutable;

    dynamic_cref(const value_storage* storage,
                 instruction_cptr)
      : field_cref(storage, storage->of_templateref.of_instruction.instruction_)
    {
    }

    explicit dynamic_cref(const field_cref& other)
      : field_cref(other.storage_, other.storage_->of_templateref.of_instruction.instruction_)
    {
    }

    bool null() const
    {
      return instruction_ == 0;
    }

    const char* name() const
    {
      return instruction()->name();
    }

    uint32_t id() const
    {
      return instruction()->id();
    }

    const template_instruction* instruction() const
    {
      if (instruction_ == 0) {
        BOOST_THROW_EXCEPTION(null_dynamic_ref());
      }
      return static_cast<const template_instruction*>(instruction_);
    }

  protected:

    friend struct encoder_impl;
};

class dynamic_mref
  : public dynamic_cref
{
  public:
    typedef boost::true_type is_mutable;
    typedef mfast::allocator allocator_type;

    dynamic_mref(allocator_type*                alloc,
                 value_storage*                 storage,
                 dynamic_cref::instruction_cptr inst)
      : dynamic_cref(storage, inst)
      , alloc_(alloc)
    {
    }

    dynamic_mref(const field_mref_base& other)
      : dynamic_cref(other)
      , alloc_(other.allocator())
    {
    }

    template <typename MESSAGE_MREF>
    MESSAGE_MREF as() const
    {
      set_instruction(MESSAGE_MREF::the_instruction, true);
      return MESSAGE_MREF(alloc_, storage());
    }

    message_mref rebind(const template_instruction* inst) const
    {
      set_instruction(inst, true);
      return message_mref(alloc_, storage(), inst);
    }

    void set_instruction(const template_instruction* inst, bool construct_subfields = true) const
    {
      if (inst == this->instruction_)
        return;

      if (this->instruction_) {
        static_cast<const template_instruction*>(this->instruction_)->destruct_value(*storage(), alloc_);
      }
      inst->construct_value(*storage(), 0, alloc_, construct_subfields);
      storage()->of_templateref.of_instruction.instruction_ = inst;
      const_cast<const field_instruction*&>(instruction_) = inst;
    }

  private:
    value_storage* storage() const
    {
      return const_cast<value_storage*>(storage_);
    }

    allocator_type*       alloc_;
};

}


#endif /* end of include guard: DYNAMIC_REF_H_X549MWYP */
