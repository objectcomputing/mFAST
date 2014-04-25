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
#ifndef MESSAGE_REF_H_CZBMHN6L
#define MESSAGE_REF_H_CZBMHN6L

#include <cassert>
#include <boost/move/core.hpp>
#include "mfast/field_instructions.h"
#include "mfast/allocator.h"
#include "mfast/field_ref.h"
#include "mfast/group_ref.h"
// #include "mfast/nested_message_ref.h"
namespace mfast {

  struct fast_decoder_impl;


  template <typename AggregateCRef, typename TemplateType>
  class make_message_cref
    : public AggregateCRef
  {
  public:

    typedef const TemplateType* instruction_cptr;

    make_message_cref(const value_storage* storage_array,
                      instruction_cptr     instruction);

    make_message_cref(const make_message_cref&);

    template <typename T>
    explicit make_message_cref(const make_message_cref<T, template_instruction>& other,
                               typename boost::disable_if_c< boost::is_same<AggregateCRef, aggregate_cref>::value &&
                                                             boost::is_same<T, aggregate_cref>::value >::type* = 0)
      : AggregateCRef(other.field_storage(0), static_cast<const TemplateType*>(other.instruction()))
    {
      assert( dynamic_cast<const TemplateType*>(other.instruction()) );
    }

    explicit make_message_cref(const field_cref& cref);

    uint32_t id() const;
    const char* ns() const;
    const char* template_ns() const;
    const char* name() const;

    instruction_cptr instruction() const;
  };

  class message_cref
    : public make_message_cref<aggregate_cref, template_instruction>
  {
    typedef make_message_cref<aggregate_cref, template_instruction> base_type;

  public:
    message_cref(const value_storage* storage_array,
                 instruction_cptr     instruction);

    template <typename AggregateCRef, typename TemplateType>
    message_cref(const make_message_cref<AggregateCRef, TemplateType>& other);

    explicit message_cref(const field_cref& cref);

  };

  template <typename AggregateMRef, typename TemplateType>
  class make_message_mref
    : public AggregateMRef
  {
  public:
    typedef make_message_cref<typename AggregateMRef::cref_type, TemplateType> cref_type;
    typedef const TemplateType* instruction_cptr;

    make_message_mref(mfast::allocator* alloc,
                      value_storage*    storage_array,
                      instruction_cptr  instruction);


    make_message_mref(const make_message_mref&);


    template <typename T>
    explicit make_message_mref(const make_message_mref<T, template_instruction>& other,
                               typename boost::disable_if< boost::is_same<AggregateMRef, T> >::type* = 0)
      : AggregateMRef(other.allocator(),
                      aggregate_mref_core_access::storage_of(other),
                      static_cast<const TemplateType*>(other.instruction()))
    {
      assert( dynamic_cast<const TemplateType*>(other.instruction()) );
    }

    explicit make_message_mref(const field_mref_base& mref);

    operator cref_type () const
    {
      return cref_type(this->field_storage(0), this->instruction());
    }

    uint32_t id() const;
    const char* ns() const;
    const char* template_ns() const;
    const char* name() const;

    instruction_cptr instruction() const;
  };

  class message_mref
    : public make_message_mref<aggregate_mref, template_instruction>
  {
    typedef make_message_mref<aggregate_mref, template_instruction> base_type;

  public:
    message_mref(mfast::allocator* alloc,
                 value_storage*    storage_array,
                 instruction_cptr  instruction);

    explicit message_mref(const field_mref_base& mref);

    template <typename AggregateMRef, typename TemplateType>
    message_mref(const make_message_mref<AggregateMRef, TemplateType>& other);

    operator message_cref ()
    {
      return message_cref(this->field_storage(0), this->instruction());
    }
  };

  template <>
  struct mref_of<message_cref>
  {
    typedef message_mref type;
  };

////////////////////////////
  template <typename AggregateCRef, typename TemplateType>
  inline
  make_message_cref<AggregateCRef,TemplateType>::make_message_cref(const value_storage* storage_array,
                                                                   typename make_message_cref<AggregateCRef,TemplateType>::instruction_cptr instruction)
    : AggregateCRef(storage_array, instruction)
  {
  }

  template <typename AggregateCRef, typename TemplateType>

  inline
  make_message_cref<AggregateCRef,TemplateType>::make_message_cref(const make_message_cref<AggregateCRef,TemplateType>& other)
    : AggregateCRef(other)
  {
  }

  // template <typename AggregateCRef, typename TemplateType>
  // template <typename T>
  // inline
  // make_message_cref(const make_message_cref<T, template_instruction>& other,
  //                   typename boost::disable_if_c< boost::is_same<AggregateCRef, aggregate_cref>::value &&
  //                                                 boost::is_same<T, aggregate_cref>::value >::type* = 0)
  //   : AggregateCRef(other.field_storage(0), other.instruction())
  // {
  //   assert( dynamic_cast<const TemplateType*>(other.instruction()) );
  // }

  template <typename AggregateCRef, typename TemplateType>
  inline
  make_message_cref<AggregateCRef,TemplateType>::make_message_cref(const field_cref &cref)
    : AggregateCRef(cref)
  {
    assert( dynamic_cast<const TemplateType*>(cref.instruction()) );
  }

  template <typename AggregateCRef, typename TemplateType>
  inline typename make_message_cref<AggregateCRef,TemplateType>::instruction_cptr
  make_message_cref<AggregateCRef,TemplateType>::instruction() const
  {
    return static_cast<instruction_cptr>(AggregateCRef::instruction());
  }

  template <typename AggregateCRef, typename TemplateType>
  inline uint32_t
  make_message_cref<AggregateCRef,TemplateType>::id() const
  {
    return this->instruction()->id();
  }

  template <typename AggregateCRef, typename TemplateType>
  inline const char*
  make_message_cref<AggregateCRef,TemplateType>::ns() const
  {
    return this->instruction()->ns();
  }

  template <typename AggregateCRef, typename TemplateType>
  inline const char*
  make_message_cref<AggregateCRef,TemplateType>::template_ns() const
  {
    return this->instruction()->template_ns();
  }

  template <typename AggregateCRef, typename TemplateType>
  inline const char*
  make_message_cref<AggregateCRef,TemplateType>::name() const
  {
    return this->instruction()->name();
  }

//////////////////////////////////////////////////////////////////
  inline
  message_cref::message_cref(const value_storage* storage_array,
                             instruction_cptr     instruction)
    : base_type(storage_array, instruction)
  {
  }

  template <typename AggregateCRef, typename TemplateType>
  inline
  message_cref::message_cref(const make_message_cref<AggregateCRef, TemplateType>& other)
    : base_type(other.field_storage(0), other.instruction())
  {
  }

  inline
  message_cref::message_cref(const field_cref& cref)
    : base_type(cref)
  {
  }

//////////////////////////////////////////////////////////////////

  template <typename AggregateMRef, typename TemplateType>
  inline
  make_message_mref<AggregateMRef,TemplateType>::make_message_mref(mfast::allocator* alloc,
                                                                   value_storage* storage_array,
                                                                   typename make_message_mref<AggregateMRef,TemplateType>::instruction_cptr instruction)
    : AggregateMRef(alloc, storage_array, instruction)
  {
  }

  template <typename AggregateMRef, typename TemplateType>
  inline
  make_message_mref<AggregateMRef,TemplateType>::make_message_mref(const make_message_mref<AggregateMRef,TemplateType>& mref)
    : AggregateMRef(mref)
  {
  }

  // template <typename AggregateMRef, typename TemplateType>
  // inline
  // make_message_mref<AggregateMRef,TemplateType>::make_message_mref(const make_message_mref<aggregate_mref, template_instruction>& mref)
  //   : AggregateMRef(other.allocator(),
  //                   const_cast<value_storage*>(other.field_storage(0)),
  //                   other.instruction())
  // {
  //   assert( dynamic_cast<const TemplateType*>(other.instruction()) );
  // }

  template <typename AggregateMRef, typename TemplateType>
  inline
  make_message_mref<AggregateMRef,TemplateType>::make_message_mref(const field_mref_base &mref)
    : AggregateMRef(mref)
  {
    assert( dynamic_cast<const TemplateType*>(mref.instruction()) );
  }

  template <typename AggregateMRef, typename TemplateType>
  inline typename make_message_mref<AggregateMRef,TemplateType>::instruction_cptr
  make_message_mref<AggregateMRef,TemplateType>::instruction() const
  {
    return static_cast<instruction_cptr>(AggregateMRef::instruction());
  }

  template <typename AggregateMRef, typename TemplateType>
  inline uint32_t
  make_message_mref<AggregateMRef,TemplateType>::id() const
  {
    return this->instruction()->id();
  }

  template <typename AggregateMRef, typename TemplateType>
  inline const char*
  make_message_mref<AggregateMRef,TemplateType>::ns() const
  {
    return this->instruction()->ns();
  }

  template <typename AggregateMRef, typename TemplateType>
  inline const char*
  make_message_mref<AggregateMRef,TemplateType>::template_ns() const
  {
    return this->instruction()->template_ns();
  }

  template <typename AggregateMRef, typename TemplateType>
  inline const char*
  make_message_mref<AggregateMRef,TemplateType>::name() const
  {
    return this->instruction()->name();
  }

/////////////////////////////////////////////////////////////////

  inline
  message_mref::message_mref(mfast::allocator* alloc,
                             value_storage*    storage_array,
                             instruction_cptr  instruction)
    : base_type(alloc,
                storage_array,
                instruction)
  {
  }

  inline
  message_mref::message_mref(const field_mref_base& mref)
    : base_type(mref)
  {
  }

  template <typename AggregateMRef, typename TemplateType>
  inline
  message_mref::message_mref(const make_message_mref<AggregateMRef, TemplateType>& other)
    : base_type(other.allocator(),
                aggregate_mref_core_access::storage_of(other),
                other.instruction())
  {
  }

}
#endif /* end of include guard: MESSAGE_REF_H_CZBMHN6L */
