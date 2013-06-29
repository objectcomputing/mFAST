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
#ifndef MESSAGE_BASE_H_CZBMHN6L
#define MESSAGE_BASE_H_CZBMHN6L

#include <cassert>
#include <boost/move/core.hpp>
#include "mfast/field_instruction.h"
#include "mfast/allocator.h"
#include "mfast/field_ref.h"
#include "mfast/group_ref.h"
namespace mfast {

// forwared declaration;
template <typename ConstMessageRef>
class make_message_mref;

class message_base;
class decoder_visitor;
class message_cref
{
  public:

    typedef boost::false_type is_mutable;
    typedef boost::false_type canbe_optional;
    typedef const template_instruction* instruction_cptr;

    message_cref(const value_storage_t* storage,
                 instruction_cptr       instruction);

    uint32_t id() const;

    const char* ns() const;

    const char* template_ns() const;

    const char* name() const;

    size_t fields_count() const;

    field_cref const_field(size_t index) const;

    /// return -1 if no such field is found
    int field_index_with_id(size_t id) const;

    /// return -1 if no such field is found
    int field_index_with_name(const char* name) const;

    const template_instruction* instruction() const;

    const field_instruction* subinstruction(size_t index) const;

    template <typename FieldAccesor>
    void accept_accessor(FieldAccesor&) const;

  protected:


    friend class decoder_visitor;

    const template_instruction* instruction_;
    const value_storage_t* storage_;

    const value_storage_t* field_storage(size_t index) const;

    // const value_storage_t* storage_for(const message_cref& other) const;
    friend class message_base;
};

template <typename ConstMessageRef>
class make_message_mref
  : public make_group_mref<ConstMessageRef>
{
  typedef make_group_mref<ConstMessageRef> base_type;

  public:
    typedef boost::true_type is_mutable;
    typedef typename base_type::instruction_cptr instruction_cptr;

    make_message_mref(allocator*       alloc,
                      value_storage_t* storage,
                      instruction_cptr instruction);



    template <typename FieldMutator>
    void accept_mutator(FieldMutator&) const;
};

typedef make_message_mref<message_cref> message_mref;


class message_base
{
  BOOST_MOVABLE_BUT_NOT_COPYABLE(message_base)

  public:
    message_base(allocator*                  alloc=0,
                 const template_instruction* instruction=0);

    // a special constructor to facilitate puting a message_base instance in an associative container
    // using emplace()
    message_base(std::pair<allocator*,const template_instruction*> p);
    ~message_base();

    message_base(const message_cref& other,
                 allocator*          alloc);

    message_base(BOOST_RV_REF(message_base)other)
      : alloc_(other.alloc_)
      , instruction_ (other.instruction_)
    {
      // g++ 4.7.1 doesn't allow this member function to defined out of class declaration
      my_storage_ = other.my_storage_;
      other.instruction_ = 0;
    }

    message_base& operator = (BOOST_RV_REF(message_base)other)
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

    message_mref ref();
    message_cref ref() const;
    message_cref cref() const;

    const template_instruction* instruction() const;
    const char* name() const;

  protected:
    const value_storage_t* storage_for(const message_cref& other) const;

    friend struct decoder_impl;

    // Used by decoder to indicate this object uses arena allocator,
    // and the allocator has been resetted. All previously allocated memory
    // are invalidated. Thus memory for sub-fields needs to be re-allocated.
    void reset();

    void ensure_valid();

    allocator*                  alloc_;
    const template_instruction* instruction_;
    value_storage_t my_storage_;
};



////////////////////////////

inline
message_cref::message_cref(const value_storage_t*      storage,
                           const template_instruction* instruction)
  : instruction_(instruction)
  , storage_(storage)
{
}

inline size_t
message_cref::fields_count() const
{
  return instruction_->subinstructions_count_;
}

inline uint32_t
message_cref::id() const
{
  return instruction_->id();
}

inline const char*
message_cref::ns() const
{
  return instruction_->ns();
}

inline const char*
message_cref::template_ns() const
{
  return instruction_->template_ns();
}

inline const char*
message_cref::name() const
{
  return instruction_->name();
}

inline field_cref
message_cref::const_field(size_t index) const
{
  assert(index < fields_count());
  return field_cref(field_storage(index),instruction_->subinstructions_[index]);
}

inline const value_storage_t*
message_cref::field_storage(size_t index) const
{
  return &storage_->group_storage.content_[index];
}

/// return -1 if no such field is found
inline int
message_cref::field_index_with_id(size_t id) const
{
  return instruction_->find_subinstruction_index_by_id(id);
}

/// return -1 if no such field is found
inline int
message_cref::field_index_with_name(const char* name) const
{
  return instruction_->find_subinstruction_index_by_name(name);
}

inline const template_instruction*
message_cref::instruction() const
{
  return instruction_;
}

inline const field_instruction*
message_cref::subinstruction(size_t index) const
{
  return instruction()->subinstruction(index);
}


///////////////////////////////////////////////////////

template <typename ConstMessageRef>
inline
make_message_mref<ConstMessageRef>::make_message_mref(allocator*                                           alloc,
                                                      value_storage_t*                                     storage,
                                                      typename make_message_mref<ConstMessageRef>::instruction_cptr instruction)
  : base_type(alloc, storage, instruction)
{
}

///////////////////////////////////////////////////////

inline
message_base::message_base(allocator*                  alloc,
                           const template_instruction* instruction)
  : alloc_(alloc)
  , instruction_(instruction)
{
  instruction_->construct_value(my_storage_, 0, alloc_);
}

inline
message_base::message_base(std::pair<allocator*,const template_instruction*> p)
  : alloc_(p.first)
  , instruction_(p.second)
{
  instruction_->construct_value(my_storage_, 0, alloc_);
}

inline
message_base::~message_base()
{
  if (this->instruction())
    this->instruction()->destruct_value(my_storage_, alloc_);
}

inline const template_instruction*
message_base::instruction() const
{
  return instruction_;
}

inline
message_base::message_base(const message_cref& other,
                           allocator*          alloc)
  : alloc_(alloc)
  , instruction_(other.instruction())
{
  this->instruction()->copy_construct_value(my_storage_, 0, alloc, other.storage_);
}


inline message_mref
message_base::ref()
{
  return message_mref(alloc_, &my_storage_, instruction_);
}

inline message_cref
message_base::ref() const
{
  return message_cref(&my_storage_, instruction_);
}

inline message_cref
message_base::cref() const
{
  return message_cref(&my_storage_, instruction_);
}

inline const value_storage_t*
message_base::storage_for(const message_cref& other) const
{
  return other.storage_;
}

inline void
message_base::reset()
{
  my_storage_.group_storage.content_ = 0;
}

inline const char*
message_base::name() const
{
  return instruction_->name();
}

inline void message_base::ensure_valid()
{
  this->instruction()->ensure_valid_storage(my_storage_, this->alloc_);
}

}
#endif /* end of include guard: MESSAGE_BASE_H_CZBMHN6L */
