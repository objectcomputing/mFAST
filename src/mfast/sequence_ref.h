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
#ifndef SEQUENCE_REF_H_OBRQQWSC
#define SEQUENCE_REF_H_OBRQQWSC

#include "mfast/field_instruction.h"
#include "mfast/field_ref.h"
#include "mfast/group_ref.h"
#include "mfast/aggregate_ref.h"
#include <cassert>


namespace mfast {


class sequence_element_cref
  : public aggregate_cref
{
  public:
    typedef const sequence_field_instruction* instruction_cptr;

    sequence_element_cref(const value_storage* storage,
                          instruction_cptr     instruction);

    sequence_element_cref(const sequence_element_cref& other);
    instruction_cptr instruction() const;

};

typedef make_aggregate_mref<sequence_element_cref> sequence_element_mref;

template <typename ElementType>
class make_sequence_cref
  : public field_cref
{
  public:
    typedef typename ElementType::instruction_cptr instruction_cptr;

    typedef ElementType reference;

    make_sequence_cref()
    {
    }

    make_sequence_cref(const value_storage* storage,
                       instruction_cptr     instruction)
      : field_cref(storage, instruction)
    {
    }

    explicit make_sequence_cref(const field_cref& other)
      : field_cref(other)
    {
    }

    reference
    operator [](size_t index) const
    {
      assert(index < size());
      //const value_storage* storages =
      //  static_cast<const value_storage*>(storage_->of_array.content_);
      return reference(element_storage(index), this->instruction());
    }

    uint32_t length() const
    {
      return size();
    }

    uint32_t size() const
    {
      return storage_->array_length();
    }

    size_t num_fields() const
    {
      return instruction()->subinstructions_count_;
    }

    instruction_cptr instruction() const
    {
      return static_cast<instruction_cptr>(instruction_);
    }

    template <typename FieldAccesor>
    void accept_accessor(FieldAccesor&) const;

  protected:
    const field_instruction* subinstruction(std::size_t index) const
    {
      return instruction()->subinstruction(index);
    }

    const value_storage* element_storage(std::size_t index) const
    {
      const value_storage* storages =
        static_cast<const value_storage*>(storage_->of_array.content_);
      return &storages[index*num_fields()];
    }

};

typedef make_sequence_cref<sequence_element_cref> sequence_cref;

namespace detail {
struct MFAST_EXPORT sequence_mref_helper
{

  static void reserve(const sequence_field_instruction* instruction,
                      value_storage*                    storage,
                      allocator*                        alloc,
                      std::size_t                       n);
};

}

template <typename ElementType>
class make_sequence_mref
  : public make_field_mref<make_sequence_cref<typename ElementType::cref_type> >
{
  typedef make_field_mref<make_sequence_cref<typename ElementType::cref_type> > base_type;

  public:
    typedef typename ElementType::instruction_cptr instruction_cptr;
    typedef ElementType reference;

    make_sequence_mref()
    {
    }

    make_sequence_mref(mfast::allocator* alloc,
                       value_storage*    storage,
                       instruction_cptr  instruction)
      : base_type(alloc, storage, instruction)
    {
    }

    make_sequence_mref(value_storage*    storage,
                       mfast::allocator* alloc)
      : base_type(storage,alloc)
    {
    }

    explicit make_sequence_mref(const field_mref_base& other)
      : base_type(other)
    {
    }

    reference
    operator [](size_t index) const
    {
      assert(index < this->size());
      return reference(this->alloc_,
                       element_storage(index),
                       this->instruction());
    }

    void resize(size_t n) const;
    void reserve(size_t n) const;


    template <typename FieldMutator>
    void accept_mutator(FieldMutator&) const;

    instruction_cptr instruction() const
    {
      return static_cast<instruction_cptr>(base_type::instruction());
    }

  private:
    value_storage* element_storage(std::size_t index) const
    {
      return const_cast<value_storage*>(base_type::element_storage(index));
    }

};

typedef make_sequence_mref<sequence_element_mref> sequence_mref;


//////////////////////////////////////////////////////////////

inline
sequence_element_cref::sequence_element_cref(const value_storage*                    storage,
                                             sequence_element_cref::instruction_cptr instruction)
  : aggregate_cref(storage, instruction)
{
}

inline
sequence_element_cref::sequence_element_cref(const sequence_element_cref &other)
  : aggregate_cref(other)
{
}

inline sequence_element_cref::instruction_cptr
sequence_element_cref::instruction() const
{
  return static_cast<instruction_cptr>(aggregate_cref::instruction());
}

template <typename ElementType>
inline void
make_sequence_mref<ElementType>::resize(size_t n) const
{
  this->reserve (n);
  this->storage()->array_length(n);
}

template <typename ElementType>
inline void
make_sequence_mref<ElementType>::reserve(size_t n) const
{
  detail::sequence_mref_helper::reserve(static_cast<const sequence_field_instruction*>(this->instruction()), this->storage(), this->alloc_, n);
}

}

#endif /* end of include guard: SEQUENCE_REF_H_OBRQQWSC */
