// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.
#pragma once

#include "field_instructions.h"
#include "field_ref.h"
#include "field_mref.h"
#include "aggregate_ref.h"
#include <cassert>

namespace mfast {
class group_cref : public field_cref {
public:
  typedef const group_field_instruction *instruction_cptr;

  group_cref() {}
  group_cref(const group_cref &other) : field_cref(other) {}
  explicit group_cref(const field_cref &other) : field_cref(other) {}
  group_cref(const value_storage *storage, const field_instruction *instruction)
      : field_cref(storage, instruction) {}

  size_t num_fields() const { return instruction()->subinstructions().size(); }
  operator aggregate_cref() const;

  field_cref operator[](size_t index) const;

  /// return -1 if no such field is found
  int field_index_with_id(size_t id) const;

  /// return -1 if no such field is found
  int field_index_with_name(const char *name) const;

  const group_field_instruction *instruction() const {
    return static_cast<const group_field_instruction *>(instruction_);
  }

  const value_storage *field_storage(size_t index) const {
    return &(this->storage_->of_group.content_[index]);
  }

private:
  group_cref &operator=(const group_cref &);
};

template <typename ConstGroupRef>
class make_group_mref : public make_field_mref<ConstGroupRef> {
  typedef make_field_mref<ConstGroupRef> base_type;

public:
  typedef typename base_type::instruction_cptr instruction_cptr;

  make_group_mref() {}
  make_group_mref(const make_group_mref &other) : base_type(other) {}
  template <typename Instruction>
  make_group_mref(mfast::allocator *alloc, value_storage *storage,
                  const Instruction *instruction)
      : base_type(alloc, storage, instruction) {
    storage->present(true);
  }

  make_group_mref(value_storage *storage, mfast::allocator *alloc)
      : base_type(storage, alloc) {
    storage->present(true);
  }

  explicit make_group_mref(const field_mref_base &other) : base_type(other) {
    base_type::storage()->present(true);
  }

  field_mref operator[](size_t index) const;

  operator aggregate_mref() const;

private:
  make_group_mref &operator=(const make_group_mref &);
  friend class detail::codec_helper;
  // void ensure_valid() const;
};

typedef make_group_mref<group_cref> group_mref;

template <> struct mref_of<group_cref> { typedef group_mref type; };

///////////////////////////////////////////////////////////////////////////////

inline group_cref::operator aggregate_cref() const {
  return aggregate_cref(
      static_cast<const value_storage *>(storage_->of_array.content_),
      instruction());
}

inline field_cref group_cref::operator[](std::size_t index) const {
  return aggregate_cref(*this)[index];
}

/// return -1 if no such field is found
inline int group_cref::field_index_with_id(std::size_t id) const {
  return aggregate_cref(*this).field_index_with_id(id);
}

inline int group_cref::field_index_with_name(const char *name) const {
  return aggregate_cref(*this).field_index_with_name(name);
}

///////////////////////////////////////////////////////////////////////////////

template <typename ConstGroupRef>
inline field_mref make_group_mref<ConstGroupRef>::
operator[](size_t index) const {
  assert(index < this->num_fields());
  return aggregate_mref(*this)[index];
}

template <typename ConstGroupRef>
inline make_group_mref<ConstGroupRef>::operator aggregate_mref() const {
  return aggregate_mref(this->alloc_, const_cast<value_storage *>(
                                          this->storage_->of_group.content_),
                        this->instruction());
}
}
