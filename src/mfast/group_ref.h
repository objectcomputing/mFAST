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
#ifndef GROUP_VALUE_REF_H_15PUY407
#define GROUP_VALUE_REF_H_15PUY407

#include "mfast/field_instruction.h"
#include "mfast/field_ref.h"
#include "mfast/field_mref.h"
#include "mfast/aggregate_ref.h"
#include <cassert>

namespace mfast {

class group_cref
  : public field_cref
{
  public:

    typedef const group_field_instruction* instruction_cptr;

    group_cref()
    {
    }

    group_cref(const group_cref& other)
      : field_cref(other)
    {
    }

    explicit group_cref(const field_cref& other)
      : field_cref(other)
    {
    }

    group_cref(const value_storage*     storage,
               const field_instruction* instruction)
      : field_cref(storage, instruction)
    {
    }

    size_t fields_count() const
    {
      return instruction()->subinstructions_count_;
    }

    operator aggregate_cref() const;

    field_cref const_field(size_t index) const;

    /// return -1 if no such field is found
    int field_index_with_id(size_t id) const;

    /// return -1 if no such field is found
    int field_index_with_name(const char* name) const;

    const group_field_instruction* instruction() const
    {
      return static_cast<const group_field_instruction*>(instruction_);
    }

    template <typename FieldAccesor>
    void accept_accessor(FieldAccesor&) const;

  protected:
    void as_present() const
    {
      if (instruction()->optional())
        const_cast<value_storage*>(storage_)->present(true);
    }

  private:
    group_cref& operator= (const group_cref&);
};


template <typename ConstGroupRef>
class make_group_mref
  : public make_field_mref<ConstGroupRef>
{
  typedef make_field_mref<ConstGroupRef> base_type;

  public:
    typedef typename base_type::instruction_cptr instruction_cptr;
    typedef ConstGroupRef cref;

    make_group_mref()
    {
    }

    make_group_mref(const make_group_mref& other)
      : base_type(other)
    {
    }

    template <typename Instruction>
    make_group_mref(mfast::allocator*  alloc,
                    value_storage*     storage,
                    const Instruction* instruction)
      : base_type(alloc, storage, instruction)
    {
    }

    make_group_mref(value_storage*    storage,
                    mfast::allocator* alloc)
      : base_type(storage, alloc)
    {
    }

    explicit make_group_mref(const field_mref_base& other)
      : base_type(other)
    {
    }

    field_mref mutable_field(size_t index) const;

    operator aggregate_mref() const;

    template <typename FieldMutator>
    void accept_mutator(FieldMutator&) const;

  private:
    make_group_mref& operator= (const make_group_mref&);
    friend class detail::codec_helper;
    void ensure_valid() const;
};

typedef make_group_mref<group_cref> group_mref;


///////////////////////////////////////////////////////////////////////////////


inline
group_cref::operator aggregate_cref() const
{
  return aggregate_cref(static_cast<const value_storage*>(storage_->of_array.content_), instruction());
}

inline field_cref
group_cref::const_field(std::size_t index) const
{
  return aggregate_cref(*this).const_field(index);
}

/// return -1 if no such field is found
inline int
group_cref::field_index_with_id(std::size_t id) const
{
  return aggregate_cref(*this).field_index_with_id(id);
}

inline int
group_cref::field_index_with_name(const char* name) const
{
  return aggregate_cref(*this).field_index_with_name(name);
}

///////////////////////////////////////////////////////////////////////////////

template <typename ConstGroupRef>
inline field_mref
make_group_mref<ConstGroupRef>::mutable_field(size_t index)  const
{
  assert(index < this->fields_count());
  this->as_present();
  // return field_mref(this->alloc_,
  //                   this->field_storage(index),
  //                   this->instruction()->subinstructions_[index]);
  return aggregate_mref(*this).mutable_field(index);
}

template <typename ConstGroupRef>
inline void
make_group_mref<ConstGroupRef>::ensure_valid() const
{
  // To improve efficiency during decoding, when the top level message is resetted,
  // all subfields' storage are zero-ed instead of properly initialized. Upon the
  // encoder visit this field, we need to check if the memory for the subfields of this
  // group is allocated. If not, we need to allocate the memory for the subfields.
  this->instruction()->ensure_valid_storage(const_cast<value_storage&>(*this->storage_), this->alloc_);
}

template <typename ConstGroupRef>
inline
make_group_mref<ConstGroupRef>::operator aggregate_mref() const
{
  return aggregate_mref(this->alloc_, const_cast<value_storage*>(this->storage_->of_group.content_), this->instruction());
}

}

#endif /* end of include guard: GROUP_VALUE_REF_H_15PUY407 */
