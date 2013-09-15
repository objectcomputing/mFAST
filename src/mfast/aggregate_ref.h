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
#include "mfast/field_ref.h"
namespace mfast
{

class unbouned_templateref_error
  : public virtual boost::exception, public virtual std::exception
{
  public:
    unbouned_templateref_error()
    {
    }

};

class aggregate_cref
{
  public:
    typedef const group_field_instruction* instruction_cptr;
    typedef boost::false_type is_mutable;

    aggregate_cref(const value_storage*           storage_array,
                   const group_field_instruction* instruction);

    aggregate_cref(const aggregate_cref& other);

    aggregate_cref(const field_cref& other);

    size_t num_fields() const;

    field_cref operator[](size_t index) const;

    /// return -1 if no such field is found
    int field_index_with_id(std::size_t id) const;

    /// return -1 if no such field is found
    int field_index_with_name(const char* name) const;

    const group_field_instruction* instruction() const;

    const field_instruction* subinstruction(size_t index) const;

    const value_storage* field_storage(size_t index) const;

    template <typename FieldAccesor>
    void accept_accessor(FieldAccesor&) const;

  protected:
    aggregate_cref& operator= (const aggregate_cref&);

    value_storage* parent_storage() const
    {
      return field_storage(num_fields())->of_group.content_;
    }
    
    const value_storage* storage() const {
      return storage_array_;
    }

    const group_field_instruction* instruction_;
    const value_storage* storage_array_;

};

template <typename ConstRef>
class make_aggregate_mref
  : public ConstRef
{
  public:
    typedef boost::true_type is_mutable;
    typedef ConstRef cref_type;
    typedef typename ConstRef::instruction_cptr instruction_cptr;

    make_aggregate_mref();

    template <typename U>
    make_aggregate_mref(const make_aggregate_mref<U>& other);

    make_aggregate_mref(mfast::allocator* alloc,
                        value_storage*    storage,
                        instruction_cptr  instruction);

    make_aggregate_mref(const field_mref_base& other);

    field_mref operator[](size_t index) const;

    mfast::allocator* allocator() const;


    template <typename FieldMutator>
    void accept_mutator(FieldMutator&) const;

  protected:
    template <class FieldMutator> friend class field_mutator_adaptor;

    value_storage* field_storage(size_t index) const;

  private:
    make_aggregate_mref& operator= (const make_aggregate_mref&);
    friend struct fast_decoder_impl;
    mfast::allocator* alloc_;
};

typedef make_aggregate_mref<aggregate_cref> aggregate_mref;



template <typename T>
class make_optional_cref
  : public T
{
  public:
    make_optional_cref(const value_storage*           storage_array,
                       const group_field_instruction* instruction)
      : T(storage_array, instruction)
    {
    }

    explicit make_optional_cref(const field_cref& other)
      : T (other)
    {
    }

    bool absent () const
    {
      return this->parent_storage()->is_empty();
    }

    bool present() const
    {
      return !absent ();
    }

    bool operator ! () const
    {
      return this->absent();
    }

#ifndef BOOST_NO_CXX11_EXPLICIT_CONVERSION_OPERATORS
    explicit bool operator() const
    {
      return this->present();
    }
#endif

};

template <typename T>
class make_optional_mref
  : public T
{
  public:
    typedef typename T::instruction_cptr instruction_cptr;

    make_optional_mref(mfast::allocator*    alloc,
                       const value_storage* storage_array,
                       instruction_cptr     instruction)
      : T(alloc, storage_array, instruction)
    {
    }

    explicit make_optional_mref(const field_mref& other)
      : T(other)
    {
    }

    typedef make_optional_cref<typename T::cref_type> cref_type;

    operator cref_type() const
    {
      return cref_type(this->storage(), this->instruction());
    }

    bool absent () const
    {
      return this->parent_storage()->is_empty();
    }

    bool present() const
    {
      return !absent ();
    }

    bool operator ! () const
    {
      return this->absent();
    }

#ifndef BOOST_NO_CXX11_EXPLICIT_CONVERSION_OPERATORS
    explicit bool operator() const
    {
      return this->present();
    }
#endif

    void as_absent() const
    {
      this->parent_storage()->present(0);
    }

    void as_present() const
    {
      this->parent_storage()->present(1);
    }

    void clear() const
    {
      as_absent();
    }
    
};


/////////////////////////////////////////////////////////////////


inline
aggregate_cref::aggregate_cref(const value_storage*           storage_array,
                               const group_field_instruction* instruction)
  : instruction_(instruction)
  , storage_array_(storage_array)
{
}

inline
aggregate_cref::aggregate_cref(const aggregate_cref& other)
  : instruction_(other.instruction())
  , storage_array_(other.storage_array_)
{
}

inline
aggregate_cref::aggregate_cref(const field_cref& other)
  : storage_array_(detail::field_storage_helper::storage_ptr_of(other)->of_group.content_)
{
  if (other.instruction()->field_type() == field_type_templateref)
  {

    this->instruction_ = static_cast<const group_field_instruction*> (detail::field_storage_helper::storage_of(other).of_templateref.of_instruction.instruction_);
    if (this->instruction_ == 0)
      BOOST_THROW_EXCEPTION(unbouned_templateref_error());
    // if you hit the exeception, you have to use dynamic_nested_message_mref::rebind() to bind a valid
    // template_instruction intead. If this is a static templateRef, please use the resolved templates
    // provided by encoder or those generated by fast_type_gen.

  }
  else {
    this->instruction_ = static_cast<const group_field_instruction*> (other.instruction());
  }
}

inline size_t
aggregate_cref::num_fields() const
{
  return instruction()->subinstructions_count_;
}

inline field_cref
aggregate_cref::operator[](size_t index) const
{
  return field_cref(&storage_array_[index],subinstruction(index));
}

inline const group_field_instruction*
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
  return &storage_array_[index];
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
  : ConstRef(other)
  , alloc_(other.allocator())
{
}

template <typename ConstRef>
inline
make_aggregate_mref<ConstRef>::make_aggregate_mref(mfast::allocator*                   alloc,
                                                   value_storage*                      storage_array,
                                                   typename ConstRef::instruction_cptr instruction)
  : ConstRef(storage_array, instruction)
  , alloc_(alloc)
{
}

template <typename ConstRef>
inline
make_aggregate_mref<ConstRef>::make_aggregate_mref(const field_mref_base& other)
  : ConstRef(other)
  , alloc_(other.allocator())
{
}

template <typename ConstRef>
inline field_mref
make_aggregate_mref<ConstRef>::operator[](size_t index) const
{
  assert(index < this->num_fields());

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

}


#endif /* end of include guard: AGGREGATE_REF_H_J48H7C2R */
