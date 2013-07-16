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
#include "mfast/exceptions.h"

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
{
  public:
    typedef const templateref_instruction* instruction_cptr;

    dynamic_cref(const value_storage* storage,
                 instruction_cptr)
      : storage_(storage)
      , instruction_(storage->of_templateref.of_instruction.instruction_)
    {
    }

    template <typename MESSAGE_CREF>
    MESSAGE_CREF static_cast_as() const
    {
      return MESSAGE_CREF(storage_,
                          static_cast<typename MESSAGE_CREF::instruction_cptr>(instruction_));
    }

    template <typename MESSAGE_CREF>
    MESSAGE_CREF dynamic_cast_as() const
    {
      if (instruction_ == 0)
        throw std::bad_cast();
      return MESSAGE_CREF(storage_,
                          dynamic_cast<typename MESSAGE_CREF::instruction_cptr>(instruction_));
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
      return instruction_;
    }

  protected:

    friend struct encoder_impl;

    const value_storage* storage_;
    const template_instruction* instruction_;
};

class dynamic_mref
  : public dynamic_cref
{
  public:
    dynamic_mref(allocator*                     alloc,
                 value_storage*                 storage,
                 dynamic_cref::instruction_cptr inst)
      : dynamic_cref(storage, inst)
      , alloc_(alloc)
    {
    }

    template <typename MESSAGE_MREF>
    MESSAGE_MREF as() const
    {
      set_instruction(MESSAGE_MREF::the_instruction, true);
      return MESSAGE_MREF(alloc_, storage_);
    }

    template <typename T>
    typename boost::enable_if<typename T::is_mutable, T>::type
    static_cast_as() const
    {
      return T(alloc_, storage_, static_cast<typename T::instruction_cptr>(instruction_));
    }

    template <typename T>
    typename boost::enable_if<typename T::is_mutable, T>::type
    dynamic_cast_as() const
    {
      if (instruction_ == 0)
        throw std::bad_cast();
      return T(alloc_, storage_, dynamic_cast<typename T::instruction_cptr>(instruction_));
    }

    template <typename T>
    typename boost::disable_if<typename T::is_mutable, T>::type
    static_cast_as() const
    {
      return T(storage_, static_cast<typename T::instruction_cptr>(instruction_));
    }

    template <typename T>
    typename boost::disable_if<typename T::is_mutable, T>::type
    dynamic_cast_as() const
    {
      if (instruction_ == 0)
        throw std::bad_cast();
      return T(storage_, dynamic_cast<typename T::instruction_cptr>(instruction_));
    }

    void set_instruction(const template_instruction* inst, bool construct_subfields)
    {
      if (inst == this->instruction_)
        return;

      if (this->instruction_) {
        this->instruction_->destruct_value(*storage(), alloc_);
      }
      inst->construct_value(*storage(), 0, alloc_, construct_subfields);
      storage()->of_templateref.of_instruction.instruction_ = inst;
      instruction_ = inst;
    }

  private:
    value_storage* storage()
    {
      return const_cast<value_storage*>(storage_);
    }

    allocator*       alloc_;
};

}


#endif /* end of include guard: DYNAMIC_REF_H_X549MWYP */
