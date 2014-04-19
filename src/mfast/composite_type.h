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

#ifndef COMPOSITE_FIELD_H_EK32RZA0
#define COMPOSITE_FIELD_H_EK32RZA0

#include <boost/move/core.hpp>
#include "mfast/message_ref.h"
#include "mfast/group_ref.h"
#include <stdlib.h>

namespace mfast
{
  template <typename CRef>
  class composite_type
  {
  private:
    BOOST_MOVABLE_BUT_NOT_COPYABLE(composite_type)

  public:

    typedef typename CRef::instruction_cptr instruction_cptr;
    typedef CRef cref_type;
    typedef typename mref_of<cref_type>::type mref_type;

    composite_type(mfast::allocator* alloc=0,
                   instruction_cptr  instruction=0,
                   value_storage*    fields_storage=0);

    // a special constructor to facilitate puting a message_type instance in an associative container
    // using emplace()
    composite_type(std::pair<mfast::allocator*,instruction_cptr> p);
    ~composite_type();

    composite_type(const cref_type & other,
                   mfast::allocator* alloc);

    composite_type(BOOST_RV_REF(composite_type)other)
      : alloc_(other.alloc_)
      , instruction_ (other.instruction_)
    {
      // g++ 4.7.1 doesn't allow this member function to defined out of class declaration
      my_storage_ = other.my_storage_;
      other.instruction_ = 0;
    }

    composite_type& operator = (BOOST_RV_REF(composite_type)other)
    {
      // g++ 4.7.1 doesn't allow this member function to defined out of class declaration
      if (this->instruction())
        this->instruction()->destruct_value(my_storage_, alloc_);

      alloc_ = other.alloc_;
      instruction_ = other.instruction_;
      my_storage_ = other.my_storage_;

      other.instruction_ = 0;
      return *this;
    }

    mref_type ref();
    mref_type mref();
    cref_type ref() const;
    cref_type cref() const;

    instruction_cptr instruction() const;
    const char* name() const;
    mfast::allocator* allocator() const;

    const value_storage& storage() const
    {
      return my_storage_;
    }

  protected:
    friend class make_group_mref<group_cref>;
    // friend class make_message_mref<message_cref>;

    composite_type(mfast::allocator*    alloc,
                   instruction_cptr     instruction,
                   value_storage*       fields_storage,
                   const value_storage* other_fields_storage);

    friend struct fast_decoder_impl;

    mfast::allocator* alloc_;
    instruction_cptr instruction_;
    value_storage my_storage_;
  };

  typedef composite_type<message_cref> message_type;
  typedef composite_type<group_cref> group_type;

///////////////////////////////////////////////////////

  template <typename CRef>
  inline
  composite_type<CRef>::composite_type(mfast::allocator*                               alloc,
                                       typename composite_type<CRef>::instruction_cptr instruction,
                                       value_storage*                                  fields_storage)
    : alloc_(alloc)
    , instruction_(instruction)
  {
    if (instruction_)
      instruction_->construct_value(my_storage_, fields_storage, alloc_);
  }

  template <typename CRef>
  inline
  composite_type<CRef>::composite_type(std::pair<mfast::allocator*,typename composite_type<CRef>::instruction_cptr> p)
    : alloc_(p.first)
    , instruction_(p.second)
  {
    instruction_->construct_value(my_storage_, 0, alloc_);
  }

  template <typename CRef>
  inline
  composite_type<CRef>::composite_type(mfast::allocator*                               alloc,
                                       typename composite_type<CRef>::instruction_cptr instruction,
                                       value_storage*                                  fields_storage,
                                       const value_storage*                            other_fields_storage)
    : alloc_(alloc)
    , instruction_(instruction)
  {
    value_storage other_storage;
    other_storage.of_group.content_ = const_cast<value_storage*>(other_fields_storage);
    this->instruction()->copy_construct_value(other_storage,
                                              my_storage_,
                                              alloc,
                                              fields_storage);
  }

  template <typename CRef>
  inline
  composite_type<CRef>::~composite_type()
  {
    if (alloc_ && this->instruction())
      this->instruction()->destruct_value(my_storage_, alloc_);
  }

  template <typename CRef>
  inline typename composite_type<CRef>::instruction_cptr
  composite_type<CRef>::instruction() const
  {
    return instruction_;
  }

  template <typename CRef>
  inline
  composite_type<CRef>::composite_type(const CRef&       other,
                                       mfast::allocator* alloc)
    : alloc_(alloc)
    , instruction_(other.instruction())
  {
    value_storage other_storage;
    other_storage.of_group.content_ = const_cast<value_storage*>(other.field_storage(0));
    this->instruction()->copy_construct_value(other_storage,
                                              my_storage_,
                                              alloc,
                                              0);
  }

  template <typename CRef>
  inline typename composite_type<CRef>::mref_type
  composite_type<CRef>::ref()
  {
    return mref_type(alloc_, my_storage_.of_group.content_, instruction_);
  }

  template <typename CRef>
  inline typename composite_type<CRef>::mref_type
  composite_type<CRef>::mref()
  {
    return mref_type(alloc_, my_storage_.of_group.content_, instruction_);
  }

  template <typename CRef>
  inline typename composite_type<CRef>::cref_type
  composite_type<CRef>::ref() const
  {
    return cref_type(my_storage_.of_group.content_, instruction_);
  }

  template <typename CRef>
  inline typename composite_type<CRef>::cref_type
  composite_type<CRef>::cref() const
  {
    return cref_type(my_storage_.of_group.content_, instruction_);
  }

  template <typename CRef>
  inline const char*
  composite_type<CRef>::name() const
  {
    return instruction_->name();
  }

  template <typename CRef>
  inline mfast::allocator*
  composite_type<CRef>::allocator() const
  {
    return this->alloc_;
  }
}

#endif /* end of include guard: COMPOSITE_FIELD_H_EK32RZA0 */
