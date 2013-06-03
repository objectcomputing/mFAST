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
#ifndef INT_REF_H_7FQR23WL
#define INT_REF_H_7FQR23WL

#include "mfast/field_instruction.h"
#include "mfast/field_ref.h"

namespace mfast {

namespace detail {
class codec_helper;
}

template <typename T>
class int_cref
  : public field_cref
{
  public:
    typedef T value_type;
    typedef typename instruction_trait<T,false>::type instruction_type;
    typedef const instruction_type* instruction_cptr;

    int_cref()
      : field_cref()
    {
    }

    int_cref(const value_storage_t*   storage,
             instruction_cptr instruction)
      : field_cref(storage, instruction)
    {
    }

    int_cref(const int_cref& other)
      : field_cref(other)
    {
    }

    uint32_t id() const
    {
      return instruction_->id();
    }


    // test if the field value is null
    operator T () const {
      return value();
    }

    T value() const
    {
      return *reinterpret_cast<const T*>(&storage_->uint_storage.content_);
    }

    instruction_cptr instruction() const
    {
      return static_cast<instruction_cptr>(instruction_);
    }

  protected:
    int_cref& operator = (const int_cref&);
};


typedef int_cref<int32_t> int32_cref;
typedef int_cref<uint32_t> uint32_cref;
typedef int_cref<int64_t> int64_cref;
typedef int_cref<uint64_t> uint64_cref;

class fast_istream;

template <typename T>
class int_mref
  : public make_field_mref<int_cref<T> >
{
  typedef make_field_mref<int_cref<T> > base_type;

  public:
    typedef const typename instruction_trait<T,false>::type* instruction_cptr;

    int_mref()
    {
    }

    int_mref(allocator*               alloc,
             value_storage_t*         storage,
             instruction_cptr instruction)
      : base_type(alloc, storage, instruction)
    {
    }

    int_mref(const int_mref& other)
      : base_type(other)
    {
    }

    const int_mref& operator = (T v) const
    {
      value(v);
      return *this;
    }

    void as(T v) const
    {
      value(v);
    }

    void as_initial_value() const
    {
      this->storage()->present(1);
      this->storage()->uint_storage.content_ = this->instruction()->initial_value();
    }

    const int_mref& operator ++ () const
    {
      ++value_ref();
      return *this;
    }

    T operator ++(int) const
    {
      T result = value();
      ++value_ref();
      return result;
    }

    T value() const
    {
      return *reinterpret_cast<const T*>(&this->storage()->uint_storage.content_);
    }

  protected:

    explicit int_mref(const field_mref& other)
      : base_type(other)
    {
    }

    void value(T& v) const
    {
      value_ref() = v;
    }

    T& value_ref() const
    {
      this->storage()->present(1);
      return *reinterpret_cast<T*>(&this->storage()->uint_storage.content_);
    }

    void as_present(bool present) const
    {
      this->storage()->present(present);
    }

    friend fast_istream& operator >> (fast_istream& strm, const int_mref<T>& mref);

    int_mref& operator = (const int_mref&);

    friend class mfast::detail::codec_helper;

    void copy_from(value_storage_t v) const
    {
      as(*reinterpret_cast<T*>(&v.uint_storage.content_));
    }

    void save_to(value_storage_t& v) const
    {
      v.uint_storage.content_ = this->storage()->uint_storage.content_;
      v.defined(true);
      v.present(this->present());
    }

};

typedef int_mref<int32_t> int32_mref;
typedef int_mref<uint32_t> uint32_mref;
typedef int_mref<int64_t> int64_mref;
typedef int_mref<uint64_t> uint64_mref;


};


#endif /* end of include guard: INT_REF_H_7FQR23WL */
