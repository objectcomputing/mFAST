// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.
#pragma once

#include "field_instructions.h"
#include <new>
#include <iostream>
#include <typeinfo>
#include <type_traits>
#include <boost/config.hpp>
#include <boost/exception/all.hpp>

namespace mfast {
namespace detail {
extern MFAST_EXPORT const value_storage null_storage;
}

template <typename ElementRef, bool IsElementAggregate>
struct sequence_iterator_base;

struct field_cref_core_access;

class bad_optional_access : public virtual boost::exception,
                            virtual public std::logic_error {
public:
  bad_optional_access();
};

class MFAST_EXPORT field_cref {
public:
  static const bool is_mutable = false;
  static const bool canbe_optional = true;
  typedef const field_instruction *instruction_cptr;

  field_cref() : instruction_(nullptr), storage_(&detail::null_storage) {}
  field_cref(const value_storage *storage, const field_instruction *instruction)
      : instruction_(instruction), storage_(storage) {}

  field_cref(const field_cref &other)
      : instruction_(other.instruction_), storage_(other.storage_) {}

  template <typename T> explicit field_cref(const T &ref);
  bool absent() const {
    return instruction_ == nullptr || (optional() && storage_->is_empty());
  }
  bool present() const { return !absent(); }
  bool optional() const { return instruction_->optional(); }
  field_type_enum_t field_type() const { return instruction_->field_type(); }
  uint32_t id() const {
    if (field_type() != field_type_templateref)
      return instruction_->id();
    if (storage_->of_templateref.of_instruction.instruction_)
      return storage_->of_templateref.of_instruction.instruction_->id();
    return 0;
  }

  const char *name() const { return instruction_->name(); }
  const field_instruction *instruction() const { return instruction_; }
  template <typename FieldAccesor> void accept_accessor(FieldAccesor &) const;

  void refers_to(const field_cref &other) {
    this->instruction_ = other.instruction_;
    this->storage_ = other.storage_;
  }

protected:
  const value_storage *storage() const { return storage_; }
  const field_instruction *instruction_;
  const value_storage *storage_;

  friend struct field_cref_core_access;

  template <typename ElementRef, bool IsElementAggregate>
  friend struct sequence_iterator_base;

private:
  field_cref &operator=(const field_cref &);
};

struct field_cref_core_access {
  static const value_storage *storage_of(const field_cref &ref) {
    return ref.storage();
  }
};

template <typename T>
field_cref::field_cref(const T &ref)
    : instruction_(ref.instruction()),
      storage_(field_cref_core_access::storage_of(ref)) {}

//////////////////////////////////////////////////////////////////

namespace detail {
template <typename T, bool CanBeEmpty> class make_field_mref_base {
protected:
  void omit() const {}
};

template <typename T> class make_field_mref_base<T, true> {
private:
  const field_instruction *my_instruction() const {
    return static_cast<const T *>(this)->instruction();
  }

  value_storage *my_storage() const {
    return static_cast<const T *>(this)->storage();
  }

public:
  // overloading void present(bool) is not a good ideal. It causes the bool
  // present()
  // declared in field_cref being hided because of the overloading rule.
  void omit() const {
    if (my_instruction()->optional()) {
      my_storage()->present(0);
    }
  }

  void clear() const {
    if (my_instruction()->optional()) {
      my_storage()->present(0);
    } else if (my_instruction()->is_array()) {
      my_storage()->array_length(0);
    }
  }
};
}

struct field_mref_core_access;

template <typename ConstFieldRef>
class make_field_mref
    : public ConstFieldRef,
      public detail::make_field_mref_base<make_field_mref<ConstFieldRef>,
                                          ConstFieldRef::canbe_optional> {
public:
  static const bool is_mutable = true;
  typedef typename ConstFieldRef::instruction_cptr instruction_cptr;
  typedef ConstFieldRef cref_type;
  typedef mfast::allocator allocator_type;

  make_field_mref() {}
  make_field_mref(allocator_type *alloc, value_storage *storage,
                  instruction_cptr instruction)
      : ConstFieldRef(storage, instruction), alloc_(alloc) {}

  make_field_mref(const make_field_mref<field_cref> &other)
      : ConstFieldRef(other), alloc_(other.alloc_) {}

  allocator_type *allocator() const { return alloc_; }
  void refers_to(const make_field_mref &other) {
    ConstFieldRef::refers_to(other);
    this->alloc_ = other.alloc_;
  }

protected:
  friend struct field_mref_core_access;

  value_storage *storage() const {
    return const_cast<value_storage *>(this->storage_);
  }
  allocator_type *alloc_;

  friend class detail::make_field_mref_base<make_field_mref<ConstFieldRef>,
                                            ConstFieldRef::canbe_optional>;

  template <typename T> friend class make_field_mref;
};

struct field_mref_core_access : field_cref_core_access {
  using field_cref_core_access::storage_of;

  template <typename T>
  static value_storage *storage_of(const make_field_mref<T> &ref) {
    return ref.storage();
  }
};

typedef make_field_mref<field_cref> field_mref_base;

template <typename T> struct cref_of { typedef typename T::cref_type type; };

template <typename T> struct mref_of;

template <typename T1, typename T2>
enable_if_t<!T1::is_mutable, T1> dynamic_cast_as(const T2 &ref) {
  typename T1::instruction_cptr instruction =
      dynamic_cast<typename T1::instruction_cptr>(ref.instruction());
  if (instruction == nullptr)
    throw std::bad_cast();
  return T1(field_mref_core_access::storage_of(ref), instruction);
}

template <typename T1, typename T2>
enable_if_t<T1::is_mutable, T1> dynamic_cast_as(const T2 &ref) {
  typename T1::instruction_cptr instruction =
      dynamic_cast<typename T1::instruction_cptr>(ref.instruction());
  if (instruction == 0)
    throw std::bad_cast();
  return T1(ref.allocator(), field_mref_core_access::storage_of(ref),
            instruction);
}

namespace detail {
inline field_cref field_ref_with_id(const value_storage *storage,
                                    const group_field_instruction *helper,
                                    uint32_t id) {
  if (helper) {
    int index = helper->find_subinstruction_index_by_id(id);
    if (index >= 0)
      return field_cref(&storage[index], helper->subinstruction(index));
  }
  return field_cref();
}

inline field_cref field_ref_with_name(const value_storage *storage,
                                      const group_field_instruction *helper,
                                      const char *name) {
  if (helper) {
    int index = helper->find_subinstruction_index_by_name(name);
    if (index >= 0)
      return field_cref(&storage[index], helper->subinstruction(index));
  }
  return field_cref();
}
}
}
