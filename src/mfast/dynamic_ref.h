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

#include "field_instruction.h"

namespace mfast
{
class dynamic_cref
{
  public:
    typedef const templateref_instruction* instruction_cptr;

    dynamic_cref(const value_storage_t* storage,
                 instruction_cptr)
      : storage_(storage)
    {
    }

    template <typename MESSAGE_CREF>
    MESSAGE_CREF static_cast_as() const
    {
      return MESSAGE_CREF(storage_,
                          static_cast<typename MESSAGE_CREF::instruction_cptr>(this->instruction()));
    }

    template <typename MESSAGE_CREF>
    MESSAGE_CREF dynamic_cast_as() const
    {
      return MESSAGE_CREF(storage_,
                          dynamic_cast<typename MESSAGE_CREF::instruction_cptr>(this->instruction()));
    }

    bool null() const
    {
      return this->instruction() == 0;
    }

  protected:
    const template_instruction* instruction() const
    {
      return storage_->templateref_storage.instruction_storage.instruction_;
    }

    const value_storage_t* storage_;
};

class dynamic_mref
  : public dynamic_cref
{
  public:
    dynamic_mref(allocator*       alloc,
                 value_storage_t* storage,
                 instruction_cptr inst)
      : dynamic_cref(storage, inst), alloc_(alloc)
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
      return T(alloc_, storage_, static_cast<typename T::instruction_cptr>(this->instruction()));
    }

    template <typename T>
    typename boost::enable_if<typename T::is_mutable, T>::type
    dynamic_cast_as() const
    {
      return T(alloc_, storage_, dynamic_cast<typename T::instruction_cptr>(this->instruction()));
    }

    template <typename T>
    typename boost::disable_if<typename T::is_mutable, T>::type
    static_cast_as() const
    {
      return T(storage_, static_cast<typename T::instruction_cptr>(this->instruction()));
    }

    template <typename T>
    typename boost::disable_if<typename T::is_mutable, T>::type
    dynamic_cast_as() const
    {
      return T(storage_, dynamic_cast<typename T::instruction_cptr>(this->instruction()));
    }

    void set_instruction(const template_instruction* inst, bool construct_subfields)
    {
      if (inst == this->instruction())
        return;

      if (this->instruction()) {
        this->instruction()->destruct_value(*storage(), alloc_);
      }
      inst->construct_value(*storage(), 0, alloc_, construct_subfields);
      storage()->templateref_storage.instruction_storage.instruction_ = inst;
    }

  private:
    value_storage_t* storage()
    {
      return const_cast<value_storage_t*>(storage_);
    }

    allocator*       alloc_;
};

}


#endif /* end of include guard: DYNAMIC_REF_H_X549MWYP */
