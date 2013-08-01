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
    
    const value_storage* storage () const
    {
      return storage_;
    }

    field_cref& operator = (const field_cref&);

    const field_instruction* instruction_;
    const value_storage* storage_;

    friend class mfast::detail::field_storage_helper;
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
    friend class detail::field_storage_helper;

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

class field_storage_helper
{
  public:
    template <typename Ref>
    static value_storage& storage_of(const Ref& ref)
    {
      return *const_cast<value_storage*>(ref.storage());
    }
    
    template <typename Ref>
    static value_storage* storage_ptr_of(const Ref& ref)
    {
      return const_cast<value_storage*>(ref.storage());
    }
    
    template <typename Ref>
    value_storage* field_storage(const Ref& ref, std::size_t i)
    {
      return ref.field_storage(i);
    }
};

}


template <typename T1, typename T2>
typename boost::disable_if<typename T1::is_mutable, T1>::type
dynamic_cast_as(const T2& ref) 
{
  typename T1::instruction_cptr instruction = dynamic_cast<typename T1::instruction_cptr>(ref.instruction());
  if (instruction == 0)
    throw std::bad_cast();
  return T1(detail::field_storage_helper::storage_ptr_of(ref), instruction);
}

template <typename T1, typename T2>
typename boost::enable_if<typename T1::is_mutable, T1>::type
dynamic_cast_as(const T2& ref)
{
  typename T1::instruction_cptr instruction = dynamic_cast<typename T1::instruction_cptr>(ref.instruction());
  if (instruction == 0)
    throw std::bad_cast();
  return T1(ref.allocator(), detail::field_storage_helper::storage_ptr_of(ref), instruction);
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
