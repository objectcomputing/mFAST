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
#ifndef AGGREGATE_REF_H_J48H7C2R
#define AGGREGATE_REF_H_J48H7C2R

#include "mfast/field_instruction.h"

namespace mfast
{
class aggregate_cref
{
  public:
    typedef boost::false_type is_mutable;
    // typedef boost::false_type canbe_optional;
    typedef const aggregate_instruction_base* instruction_cptr;

    aggregate_cref(const value_storage* storage,
                   instruction_cptr     instruction);

    aggregate_cref(const aggregate_cref& other);
    
    size_t fields_count() const;

    field_cref const_field(size_t index) const;

    /// return -1 if no such field is found
    int field_index_with_id(std::size_t id) const;

    /// return -1 if no such field is found
    int field_index_with_name(const char* name) const;

    const aggregate_instruction_base* instruction() const;

    const field_instruction* subinstruction(size_t index) const;

    const value_storage* field_storage(size_t index) const;
    
    template <typename FieldAccesor>
    void accept_accessor(FieldAccesor&) const;

  protected:
    aggregate_cref& operator= (const aggregate_cref&);

    const value_storage* storage() const
    {
      return storage_;
    }
    const aggregate_instruction_base* instruction_;
    const value_storage* storage_;
    
    friend class detail::field_storage_helper;
};

template <typename ConstRef>
class make_aggregate_mref
  : public ConstRef
{
  public:
    typedef boost::true_type is_mutable;   
    typedef typename ConstRef::instruction_cptr instruction_cptr;
    typedef ConstRef cref;

    make_aggregate_mref();

    template <typename U>
    make_aggregate_mref(const make_aggregate_mref<U>& other);

    make_aggregate_mref(mfast::allocator* alloc,
                        value_storage*    storage,
                        instruction_cptr  instruction);

    field_mref mutable_field(size_t index) const;

    mfast::allocator* allocator() const;
    
    
    template <typename FieldMutator>
    void accept_mutator(FieldMutator&) const;
    
  protected:
    template <class FieldMutator> friend class field_mutator_adaptor;

    value_storage* field_storage(size_t index) const;

  private:
    make_aggregate_mref& operator= (const make_aggregate_mref&);
    friend struct decoder_impl;
    void ensure_valid() const;

    mfast::allocator* alloc_;
    friend class detail::field_storage_helper;
};

typedef make_aggregate_mref<aggregate_cref> aggregate_mref;

/////////////////////////////////////////////////////////////////


inline
aggregate_cref::aggregate_cref(const value_storage* storage,
                               instruction_cptr     instruction)
  : instruction_(instruction)
  , storage_(storage)
{
}

inline
aggregate_cref::aggregate_cref(const aggregate_cref& other)
  : instruction_(other.instruction_)
  , storage_(other.storage_)
{
}

inline size_t
aggregate_cref::fields_count() const
{
  return instruction()->subinstructions_count_;
}

inline field_cref
aggregate_cref::const_field(size_t index) const
{
  return field_cref(&storage_[index],subinstruction(index));
}

inline const aggregate_instruction_base*
aggregate_cref::instruction() const
{
  return instruction_;
}

inline const field_instruction*
aggregate_cref::subinstruction(size_t index) const
{
  return instruction()->subinstruction(index);
}

inline const value_storage*
aggregate_cref::field_storage(size_t index) const
{
  return &storage_[index];
}

/// return -1 if no such field is found
inline int
aggregate_cref::field_index_with_id(std::size_t id) const
{
  return instruction()->find_subinstruction_index_by_id(id);
}

/// return -1 if no such field is found
inline int
aggregate_cref::field_index_with_name(const char* name) const
{
  return instruction()->find_subinstruction_index_by_name(name);
}

///////////////////////////////////////////////////////

template <typename ConstRef>
inline
make_aggregate_mref<ConstRef>::make_aggregate_mref()
  : alloc_(0)
{
}

template <typename ConstRef>
template <typename U>
inline
make_aggregate_mref<ConstRef>::make_aggregate_mref(const make_aggregate_mref<U>& other)
  : ConstRef(detail::field_storage_helper::storage_ptr_of(other), other.instruction())
  , alloc_(other.allocator())
{
}

template <typename ConstRef>
inline
make_aggregate_mref<ConstRef>::make_aggregate_mref(mfast::allocator* alloc,
                                                   value_storage*    storage,
                                                   instruction_cptr  instruction)
  : ConstRef(storage, instruction)
  , alloc_(alloc)
{
}

template <typename ConstRef>
inline field_mref
make_aggregate_mref<ConstRef>::mutable_field(size_t index) const
{
  assert(index < this->fields_count());
  return field_mref(this->alloc_,
                    this->field_storage(index),
                    this->instruction()->subinstruction(index));
}

template <typename ConstRef>
inline mfast::allocator*
make_aggregate_mref<ConstRef>::allocator() const
{
  return alloc_;
}

template <typename ConstRef>
inline value_storage*
make_aggregate_mref<ConstRef>::field_storage(size_t index) const
{
  return const_cast<value_storage*>(ConstRef::field_storage(index));
}

template <typename ConstRef>
inline void
make_aggregate_mref<ConstRef>::ensure_valid() const
{
  // To improve efficiency during decoding, when the top level message is resetted,
  // all subfields' storage are zero-ed instead of properly initialized. Upon the
  // encoder visit this field, we need to check if the memory for the subfields of this
  // group is allocated. If not, we need to allocate the memory for the subfields.
  this->instruction()->ensure_valid_storage(const_cast<value_storage&>(*this->storage_), this->alloc_);
}



}


#endif /* end of include guard: AGGREGATE_REF_H_J48H7C2R */
