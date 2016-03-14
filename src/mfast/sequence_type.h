// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.
#pragma once
#include <boost/move/core.hpp>
#include "sequence_ref.h"
namespace mfast {
template <typename CRef> class squence_type_base {
#ifdef BOOST_NO_RVALUE_REFERENCES
  BOOST_MOVABLE_BUT_NOT_COPYABLE(squence_type_base)
#endif
public:
  typedef typename CRef::instruction_cptr instruction_cptr;
  typedef CRef cref_type;
  typedef typename mref_of<cref_type>::type mref_type;

  squence_type_base(mfast::allocator *alloc = nullptr,
                    instruction_cptr instruction = 0,
                    value_storage *fields_storage = nullptr);

  // a special constructor to facilitate puting a message_type instance in an
  // associative
  // container
  // using emplace()
  squence_type_base(std::pair<mfast::allocator *, instruction_cptr> p);
  ~squence_type_base();

  squence_type_base(const cref_type &other, mfast::allocator *alloc);

  squence_type_base(BOOST_RV_REF(squence_type_base) other)
      : alloc_(other.alloc_), instruction_(other.instruction_) {
    // g++ 4.7.1 doesn't allow this member function to defined out of class
    // declaration
    my_storage_ = other.my_storage_;
    other.instruction_ = 0;
  }

  squence_type_base &operator=(BOOST_RV_REF(squence_type_base) other) {
    // g++ 4.7.1 doesn't allow this member function to defined out of class
    // declaration
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
  const char *name() const;
  mfast::allocator *allocator() const;

  const value_storage &storage() const { return my_storage_; }

protected:
  // Used by decoder to indicate this object uses arena allocator,
  // and the allocator has been resetted. All previously allocated memory
  // are invalidated. Thus memory for sub-fields needs to be re-allocated.

  // void reset();
  // void ensure_valid();

  friend struct fast_decoder_impl;

  mfast::allocator *alloc_;
  instruction_cptr instruction_;
  value_storage my_storage_;
};

typedef squence_type_base<sequence_cref> sequence_type;

///////////////////////////////////////////////////////

template <typename CRef>
inline squence_type_base<CRef>::squence_type_base(
    mfast::allocator *alloc,
    typename squence_type_base<CRef>::instruction_cptr instruction,
    value_storage *fields_storage)
    : alloc_(alloc), instruction_(instruction) {
  if (instruction_)
    instruction_->construct_value(my_storage_, fields_storage, alloc_, false);
}

template <typename CRef>
inline squence_type_base<CRef>::squence_type_base(
    std::pair<mfast::allocator *,
              typename squence_type_base<CRef>::instruction_cptr> p)
    : alloc_(p.first), instruction_(p.second) {
  instruction_->construct_value(my_storage_, 0, alloc_, false);
}

template <typename CRef> inline squence_type_base<CRef>::~squence_type_base() {
  if (alloc_ && this->instruction())
    this->instruction()->destruct_value(my_storage_, alloc_);
}

template <typename CRef>
inline typename squence_type_base<CRef>::instruction_cptr
squence_type_base<CRef>::instruction() const {
  return instruction_;
}

template <typename CRef>
inline squence_type_base<CRef>::squence_type_base(const CRef &other,
                                                  mfast::allocator *alloc)
    : alloc_(alloc), instruction_(other.instruction()) {
  this->instruction()->copy_construct_value(
      *field_cref_core_access::storage_of(other), my_storage_, alloc, nullptr);
}

template <typename CRef>
inline typename squence_type_base<CRef>::mref_type
squence_type_base<CRef>::ref() {
  return mref_type(alloc_, &my_storage_, instruction_);
}

template <typename CRef>
inline typename squence_type_base<CRef>::mref_type
squence_type_base<CRef>::mref() {
  return mref_type(alloc_, &my_storage_, instruction_);
}

template <typename CRef>
inline typename squence_type_base<CRef>::cref_type
squence_type_base<CRef>::ref() const {
  return cref_type(&my_storage_, instruction_);
}

template <typename CRef>
inline typename squence_type_base<CRef>::cref_type
squence_type_base<CRef>::cref() const {
  return cref_type(&my_storage_, instruction_);
}

template <typename CRef>
inline const char *squence_type_base<CRef>::name() const {
  return instruction_->name();
}

template <typename CRef>
inline mfast::allocator *squence_type_base<CRef>::allocator() const {
  return this->alloc_;
}

// template <typename CRef>
// inline void
// squence_type_base<CRef>::reset()
// {
//   my_storage_.of_array.content_ = 0;
//   my_storage_.of_array.capacity_in_bytes_ = 0;
// }
//
// template <typename CRef>
// inline void
// squence_type_base<CRef>::ensure_valid()
// {
//   instruction_->ensure_valid_storage(my_storage_, alloc_);
// }

} /* mfast */
