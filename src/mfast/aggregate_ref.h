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
#ifndef AGGREGATE_REF_H_J48H7C2R
#define AGGREGATE_REF_H_J48H7C2R

#include "mfast/field_instructions.h"
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

  // forward declaration
  template <typename T>
  class make_aggregate_mref;


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


    bool absent () const
    {
      return storage_array_ == 0;
    }

    bool present() const
    {
      return !absent();
    }

//     bool operator ! () const
//     {
//       return this->absent();
//     }
//
// #ifndef BOOST_NO_CXX11_EXPLICIT_CONVERSION_OPERATORS
//     explicit operator bool() const
//     {
//       return this->present();
//     }
//
// #endif

    void refers_to(const aggregate_cref& other)
    {
      this->instruction_ = other.instruction_;
      this->storage_array_ = other.storage_array_;
    }

  protected:
    aggregate_cref& operator= (const aggregate_cref&);


    const value_storage* storage() const
    {
      return storage_array_;
    }

    const group_field_instruction* instruction_;
    const value_storage* storage_array_;

    template <typename T>
    friend class make_aggregate_mref;

    template <typename ElementRef, bool IsElementAggregate>
    friend struct sequence_iterator_base;

  };

  class message_mref;

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


    void as(const ConstRef& other) const;

    void refers_to(const make_aggregate_mref<ConstRef>& other)
    {
      ConstRef::refers_to(other);
      this->alloc_ = other.alloc_;
    }

  protected:
    template <class FieldMutator> friend class field_mutator_adaptor;

    value_storage* field_storage(size_t index) const;

  private:
    template <typename U> friend class make_aggregate_mref;

    make_aggregate_mref& operator= (const make_aggregate_mref&);
    friend struct fast_decoder_impl;
    friend class message_mref;
    mfast::allocator* alloc_;
  };

  typedef make_aggregate_mref<aggregate_cref> aggregate_mref;


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
    return instruction()->subinstructions_count();
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
    : ConstRef(other.field_storage(0), static_cast<instruction_cptr>(other.instruction()))
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

  template <typename ConstRef>
  inline void
  make_aggregate_mref<ConstRef>::as(const ConstRef& other) const
  {
    // we can only do the assignment when other is present
    assert(other.present());

    // make sure the type are exactly the same
    assert(typeid(*this->instruction()) == typeid(*other.instruction()));


    std::size_t sz = this->instruction()->group_content_byte_count();

    // first, we need to allocate two chuck of memories of size sz
    char* buf = static_cast<char*>(alloca(2 * sz));
    value_storage* orig_content = reinterpret_cast<value_storage*>(buf);
    value_storage* new_content = reinterpret_cast<value_storage*>(buf+sz);
    // the first chuck for storing the orignal content
    std::memcpy(orig_content, this->storage_array_, sz);
    value_storage tmp_storage;

    // the second chunk for construct the new aggregate value

    value_storage other_storage;
    other_storage.of_group.own_content_ = false;
    other_storage.of_group.content_ = const_cast<value_storage*>(other.storage_array_);

    this->instruction()->copy_construct_value(other_storage,
                                              tmp_storage,
                                              this->alloc_,
                                              new_content);

    // copy the newly construct aggregate value to the old memory chunk
    std::memcpy(const_cast<value_storage*>(this->storage_array_), new_content, sz);
    // destruct the subfiels on first memory chunk which contains the original value
    this->instruction()->destruct_group_subfields(orig_content, alloc_);

  }

}


#endif /* end of include guard: AGGREGATE_REF_H_J48H7C2R */
