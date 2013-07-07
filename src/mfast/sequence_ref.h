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
#include <cassert>


namespace mfast {

class sequence_element_cref
{
  public:
    typedef boost::false_type is_mutable;
    typedef boost::false_type canbe_optional;
    typedef const sequence_field_instruction* instruction_cptr;

    sequence_element_cref(const value_storage_t* storage,
                          instruction_cptr       instruction)
      : instruction_(instruction)
      , storage_(storage)
    {
    }

    sequence_element_cref(const sequence_element_cref& other)
      : instruction_(other.instruction_)
      , storage_(other.storage_)
    {
    }

    size_t fields_count() const
    {
      return instruction()->subinstructions_count_;
    }

    field_cref const_field(size_t index) const
    {
      return field_cref(&storage_[index],subinstruction(index));
    }

    /// return -1 if no such field is found
    int field_index_with_id(std::size_t id) const;

    /// return -1 if no such field is found
    int field_index_with_name(const char* name) const;

    const sequence_field_instruction* instruction() const
    {
      return instruction_;
    }

    template <typename T>
    T static_cast_as() const
    {
      return T(storage_, static_cast<typename T::instruction_cptr>(instruction_));
    }

    template <typename T>
    T dynamic_cast_as() const
    {
      return T(storage_, dynamic_cast<typename T::instruction_cptr>(instruction_));
    }

  protected:
    sequence_element_cref& operator= (const sequence_element_cref&);
    const value_storage_t* field_storage(size_t index) const;
    const field_instruction* subinstruction(size_t index) const
    {
      return instruction()->subinstruction(index);
    }

    const sequence_field_instruction* instruction_;
    const value_storage_t* storage_;
};

typedef make_group_mref<sequence_element_cref> sequence_element_mref;

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

    make_sequence_cref(const value_storage_t* storage,
                       instruction_cptr       instruction)
      : field_cref(storage, instruction)
    {
    }

    reference
    operator [](size_t index) const
    {
      assert(index < size());
      //const value_storage_t* storages =
      //  static_cast<const value_storage_t*>(storage_->array_storage.content_);
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

    size_t fields_count() const
    {
      return instruction()->subinstructions_count_;
    }

    const sequence_field_instruction* instruction() const
    {
      return static_cast<const sequence_field_instruction*>(instruction_);
    }

    template <typename FieldAccesor>
    void accept_accessor(FieldAccesor&) const;

  protected:
    const field_instruction* subinstruction(std::size_t index) const
    {
      return instruction()->subinstruction(index);
    }

    const value_storage_t* element_storage(std::size_t index) const
    {
      const value_storage_t* storages =
        static_cast<const value_storage_t*>(storage_->array_storage.content_);
      return &storages[index*fields_count()];
    }

};

typedef make_sequence_cref<sequence_element_cref> sequence_cref;

namespace detail {
struct sequence_mref_helper
{
  static void resize(const sequence_field_instruction* instruction,
                     value_storage_t*                  storage,
                     allocator*                        alloc,
                     std::size_t                       n);

  static void reserve(const sequence_field_instruction* instruction,
                      value_storage_t*                  storage,
                      allocator*                        alloc,
                      std::size_t                       n);
};

}

template <typename ElementType>
class make_sequence_mref
  : public make_field_mref<make_sequence_cref<typename ElementType::cref> >
{
  typedef make_field_mref<make_sequence_cref<typename ElementType::cref> > base_type;

  public:
    typedef typename ElementType::instruction_cptr instruction_cptr;
    typedef ElementType reference;

    make_sequence_mref()
    {
    }

    make_sequence_mref(allocator*       alloc,
                       value_storage_t* storage,
                       instruction_cptr instruction)
      : base_type(alloc, storage, instruction)
    {
    }

    make_sequence_mref(value_storage_t* storage,
                       allocator*       alloc)
      : base_type(storage,alloc)
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
    value_storage_t* element_storage(std::size_t index) const
    {
      return const_cast<value_storage_t*>(base_type::element_storage(index));
    }

};

void resize_sequence(make_sequence_mref<sequence_element_mref>& seq, std::size_t n);
void reserve_sequence(make_sequence_mref<sequence_element_mref>& seq, std::size_t n);

typedef make_sequence_mref<sequence_element_mref> sequence_mref;


//////////////////////////////////////////////////////////////

inline const value_storage_t*
sequence_element_cref::field_storage(size_t index) const
{
  return &storage_[index];
}

/// return -1 if no such field is found
inline int
sequence_element_cref::field_index_with_id(std::size_t id) const
{
  return instruction()->find_subinstruction_index_by_id(id);
}

/// return -1 if no such field is found
inline int
sequence_element_cref::field_index_with_name(const char* name) const
{
  return instruction()->find_subinstruction_index_by_name(name);
}

template <typename ElementType>
inline void
make_sequence_mref<ElementType>::resize(size_t n) const
{
  detail::sequence_mref_helper::resize(this->instruction(), this->storage(), this->alloc_, n);
}

template <typename ElementType>
inline void
make_sequence_mref<ElementType>::reserve(size_t n) const
{
  detail::sequence_mref_helper::reserve(this->instruction(), this->storage(), this->alloc_, n);
}

}

#endif /* end of include guard: SEQUENCE_REF_H_OBRQQWSC */
