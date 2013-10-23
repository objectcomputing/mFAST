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
#ifndef MESSAGE_REF_H_CZBMHN6L
#define MESSAGE_REF_H_CZBMHN6L

#include <cassert>
#include <boost/move/core.hpp>
#include "mfast/field_instruction.h"
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
  explicit make_message_cref(const field_cref& cref);

  uint32_t id() const;
  const char* ns() const;
  const char* template_ns() const;
  const char* name() const;

  instruction_cptr instruction() const;
};

typedef make_message_cref<aggregate_cref, template_instruction> message_cref;

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

// typedef make_message_mref<aggregate_mref, template_instruction> message_mref;

class message_mref
  : public make_message_mref<aggregate_mref, template_instruction>
{
public:
  message_mref(mfast::allocator* alloc,
               value_storage*    storage_array,
               instruction_cptr  instruction);

  explicit message_mref(const field_mref_base& mref);

  template <typename AggregateMRef, typename TemplateType>
  message_mref(const make_message_mref<AggregateMRef, TemplateType>& other);
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

template <typename AggregateCRef, typename TemplateType>
inline
make_message_cref<AggregateCRef,TemplateType>::make_message_cref(const field_cref &cref)
  : AggregateCRef(cref)
{
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

template <typename AggregateMRef, typename TemplateType>
inline
make_message_mref<AggregateMRef,TemplateType>::make_message_mref(const field_mref_base &mref)
  : AggregateMRef(mref)
{
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
  : make_message_mref<aggregate_mref, template_instruction>(alloc,
                                                            storage_array,
                                                            instruction)
{
}

inline
message_mref::message_mref(const field_mref_base& mref)
  : make_message_mref<aggregate_mref, template_instruction>(mref)
{
}

template <typename AggregateMRef, typename TemplateType>
inline
message_mref::message_mref(const make_message_mref<AggregateMRef, TemplateType>& other)
  : make_message_mref<aggregate_mref, template_instruction>(other.allocator(),
                                                            const_cast<value_storage*>(other.field_storage(0)),
                                                            other.instruction())
{
}

}
#endif /* end of include guard: MESSAGE_REF_H_CZBMHN6L */
