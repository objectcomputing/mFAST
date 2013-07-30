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
#ifndef FIELD_VALUE_H_BJLDKLDX
#define FIELD_VALUE_H_BJLDKLDX

#include "mfast/field_instruction.h"
#include <new>
#include <iostream>
#include <boost/config.hpp>
#include <boost/type_traits.hpp>
#include <boost/utility/enable_if.hpp>

namespace mfast {


namespace detail {
extern const value_storage null_storage;

class field_storage_helper;
}

class field_cref;
class message_cref;

class field_cref
{
  public:

    typedef boost::false_type is_mutable;
    typedef boost::true_type canbe_optional;
    typedef const field_instruction* instruction_cptr;

    field_cref()
      : instruction_(0)
      , storage_(&detail::null_storage)
    {
    }

    field_cref(const value_storage*     storage,
               const field_instruction* instruction)
      : instruction_(instruction)
      , storage_(storage)
    {
    }

    field_cref(const field_cref& other)
      : instruction_(other.instruction_)
      , storage_(other.storage_)
    {
    }

    bool absent () const
    {
      return instruction_== 0 ||  (optional() && storage_->is_empty());
    }

    bool present() const
    {
      return !absent ();
    }

    bool optional() const
    {
      return instruction_->optional();
    }

    bool has_initial_value() const
    {
      return instruction_->has_initial_value();
    }

    field_type_enum_t field_type() const
    {
      return instruction_->field_type();
    }

    uint32_t id() const
    {
      if (field_type() != field_type_templateref)
        return instruction_->id();
      if (storage_->of_templateref.of_instruction.instruction_)
        return storage_->of_templateref.of_instruction.instruction_->id();
      return 0;
    }

    const char* name() const
    {
      return instruction_->name();
    }

    const field_instruction* instruction() const
    {
      return instruction_;
    }

  protected:

    field_cref& operator = (const field_cref&);


    const value_storage* storage () const
    {
      return storage_;
    }

    const field_instruction* instruction_;
    const value_storage* storage_;

    friend class mfast::detail::field_storage_helper;
    friend class message_cref;
    friend class dynamic_message_cref;
};

//////////////////////////////////////////////////////////////////

namespace detail
{
template <typename T, typename CanBeEmpty>
class make_field_mref_base
{
  protected:
    void as_present() const
    {
    }

};

template <typename T>
class make_field_mref_base<T, boost::true_type>
{
  public:
    void as_absent() const
    {
      const T* ptr = static_cast<const T*>(this);
      if (ptr->instruction()->optional()) {
        ptr->storage()->present(0);
      }
    }

};

}


template <typename ConstFieldRef>
class make_field_mref
  : public ConstFieldRef
  , public detail::make_field_mref_base< make_field_mref<ConstFieldRef>,
                                         typename ConstFieldRef::canbe_optional >
{
  public:
    typedef boost::true_type is_mutable;
    typedef typename ConstFieldRef::instruction_cptr instruction_cptr;
    typedef ConstFieldRef cref_type;
    typedef mfast::allocator allocator_type;

    make_field_mref()
    {
    }

    make_field_mref(allocator_type*  alloc,
                    value_storage*   storage,
                    instruction_cptr instruction)
      : ConstFieldRef(storage, instruction)
      , alloc_(alloc)
    {
    }

    make_field_mref(const make_field_mref<field_cref> &other)
      : ConstFieldRef(other)
      , alloc_(other.alloc_)
    {
    }

    allocator_type* allocator() const
    {
      return alloc_;
    }

  protected:
    value_storage* storage () const
    {
      return const_cast<value_storage*>(this->storage_);
    }

    allocator_type* alloc_;

    friend class detail::make_field_mref_base< make_field_mref<ConstFieldRef>,
                                               typename ConstFieldRef::canbe_optional >;
    friend class field_mutator_adaptor_base;

    template <typename T>
    friend class make_field_mref;
};

typedef make_field_mref<field_cref> field_mref_base;

template <typename T>
struct cref_of
{
  typedef typename T::cref_type type;
};

template <typename T>
struct mref_of;




namespace detail {

template <typename T>
struct remove_const_reference;

template <typename T>
struct remove_const_reference<const T&>
{
  typedef T type;
};

template <typename T>
struct remove_const_reference<T&>
{
  typedef T type;
};

}

namespace detail {

inline field_cref
field_ref_with_id(const value_storage*        storage,
                  const group_content_helper* helper,
                  uint32_t                    id)
{
  if (helper) {

    int index = helper->find_subinstruction_index_by_id(id);
    if (index >= 0)
      return field_cref(&storage[index], helper->subinstructions_[index]);
  }
  return field_cref();
}

inline field_cref
field_ref_with_name(const value_storage*        storage,
                    const group_content_helper* helper,
                    const char*                 name)
{
  if (helper) {
    int index = helper->find_subinstruction_index_by_name(name);
    if (index >= 0)
      return field_cref(&storage[index], helper->subinstructions_[index]);
  }
  return field_cref();
}

}

}

#endif /* end of include guard: FIELD_VALUE_H_BJLDKLDX */
