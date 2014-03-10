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

#ifndef BOOLEAN_REF_H_H7MFNBMO
#define BOOLEAN_REF_H_H7MFNBMO
#include "mfast_export.h"
#include "enum_ref.h"

namespace mfast
{

  struct MFAST_EXPORT boolean
  {
    typedef enum_field_instruction_ex<boolean> instruction_type;
    static const instruction_type* instruction();
  };

  class boolean_cref
    : public enum_cref
  {
  public:
    typedef enum_cref base_type;
    typedef bool element_type;
    typedef boolean::instruction_type instruction_type;
    typedef const instruction_type* instruction_cptr;

    boolean_cref(
      const mfast::value_storage* storage=0,
      instruction_cptr            instruction=0)
      : base_type(storage, instruction)
    {
    }

    explicit boolean_cref(const field_cref& other)
      : base_type(other)
    {
    }

    bool is_false() const
    {
      return !value();
    }

    bool is_true() const
    {
      return value();
    }

    bool value() const
    {
      return base_type::value() != 0;
    }

  };

  class boolean_mref
    : public make_field_mref<boolean_cref>
  {
  public:
    typedef make_field_mref<boolean_cref> base_type;
    typedef bool element_type;
    typedef boolean::instruction_type instruction_type;
    typedef const instruction_type* instruction_cptr;

    boolean_mref(
      mfast::allocator*     alloc=0,
      mfast::value_storage* storage=0,
      instruction_cptr      instruction=0)
      : base_type(alloc, storage, instruction)
    {
    }

    explicit boolean_mref(const mfast::field_mref_base& other)
      : base_type(other)
    {
    }

    void as_false() const
    {
      this->as(false);
    }

    void as_true() const
    {
      this->as(true);
    }

    void as(bool v) const
    {
      this->storage()->present(1);
      this->storage()->set<uint64_t>(v);
    }

  };

} /* mfast */

#endif /* end of include guard: BOOLEAN_REF_H_H7MFNBMO */
