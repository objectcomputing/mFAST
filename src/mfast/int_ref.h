// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.
#pragma once

#include "field_instructions.h"
#include "field_ref.h"
#include "type_category.h"

namespace mfast {
namespace detail {
class codec_helper;
}

template <typename T> class int_cref : public field_cref {
public:
  typedef T value_type;
  typedef int_field_instruction<T> instruction_type;
  typedef const instruction_type *instruction_cptr;
  typedef integer_type_tag type_category;

  int_cref() : field_cref() {}
  int_cref(const value_storage *storage, instruction_cptr instruction)
      : field_cref(storage, instruction) {}

  int_cref(const int_cref &other) : field_cref(other) {}
  explicit int_cref(const field_cref &other) : field_cref(other) {}
  uint32_t id() const { return instruction_->id(); }
  bool is_initial_value() const {
    return (
        this->absent() == this->instruction()->initial_value().is_empty() &&
        (this->absent() ||
         value() == this->instruction()->initial_value().template get<T>()));
  }

  T value() const { return storage_->template get<T>(); }
  instruction_cptr instruction() const {
    return static_cast<instruction_cptr>(instruction_);
  }

protected:
  friend class mfast::detail::codec_helper;

  int_cref &operator=(const int_cref &);

  void save_to(value_storage &v) const {
    v.of_uint64.content_ = this->storage()->of_uint64.content_;
    v.defined(true);
    v.present(this->present());
  }
};

template <typename T>
inline bool operator==(const int_cref<T> &lhs, const int_cref<T> &rhs) {
  return (lhs.absent() == rhs.absent()) &&
         (lhs.absent() || lhs.value() == rhs.value());
}

template <typename T>
inline bool operator!=(const int_cref<T> &lhs, const int_cref<T> &rhs) {
  return !(lhs == rhs);
}

typedef int_cref<int32_t> int32_cref;
typedef int_cref<uint32_t> uint32_cref;
typedef int_cref<int64_t> int64_cref;
typedef int_cref<uint64_t> uint64_cref;

class fast_istream;

template <typename T> class int_mref : public make_field_mref<int_cref<T>> {
  typedef make_field_mref<int_cref<T>> base_type;

public:
  typedef int_field_instruction<T> instruction_type;
  typedef const instruction_type *instruction_cptr;

  int_mref() {}
  int_mref(mfast::allocator *alloc, value_storage *storage,
           instruction_cptr instruction)
      : base_type(alloc, storage, instruction) {}

  int_mref(const int_mref &) = default;

  explicit int_mref(const field_mref_base &other) : base_type(other) {}
  void as(const int_cref<T> &cref) const {
    if (cref.absent()) {
      this->omit();
    } else {
      as(cref.value());
    }
  }

  void as(T v) const {
    this->storage()->present(1);
    this->storage()->template set<T>(v);
  }

  void to_initial_value() const {
    *this->storage() = this->instruction()->initial_value();
  }
  T value() const { return this->storage()->template get<T>(); }

protected:
  // T& value_ref() const
  // {
  //   this->storage()->present(1);
  //   return *reinterpret_cast<T*>(&this->storage()->of_uint64.content_);
  // }

  void as_present(bool present) const { this->storage()->present(present); }
  template <typename U>
  friend fast_istream &operator>>(fast_istream &strm, const int_mref<U> &mref);

  int_mref &operator=(const int_mref &);

  friend class mfast::detail::codec_helper;

  void copy_from(value_storage v) const { *this->storage() = v; }
};

typedef int_mref<int32_t> int32_mref;
typedef int_mref<uint32_t> uint32_mref;
typedef int_mref<int64_t> int64_mref;
typedef int_mref<uint64_t> uint64_mref;

template <> struct mref_of<int32_cref> { typedef int32_mref type; };

template <> struct mref_of<uint32_cref> { typedef uint32_mref type; };

template <> struct mref_of<int64_cref> { typedef int64_mref type; };

template <> struct mref_of<uint64_cref> { typedef uint64_mref type; };
};
