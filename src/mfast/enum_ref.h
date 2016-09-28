// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.
#pragma once

#include "mfast/field_instructions.h"
#include "mfast/field_ref.h"
#include "mfast/type_category.h"

namespace mfast {
namespace detail {
class codec_helper;
}

class MFAST_EXPORT enum_cref : public field_cref {
public:
  typedef uint_fast32_t value_type;
  typedef enum_field_instruction instruction_type;
  typedef const instruction_type *instruction_cptr;
  typedef integer_type_tag type_category;

  enum_cref() : field_cref() {}
  enum_cref(const value_storage *storage, instruction_cptr instruction)
      : field_cref(storage, instruction) {}

  enum_cref(const enum_cref &other) : field_cref(other) {}
  explicit enum_cref(const field_cref &other) : field_cref(other) {}
  uint32_t id() const { return instruction_->id(); }
  bool is_initial_value() const {
    return (this->absent() == this->instruction()->initial_value().is_empty() &&
            (this->absent() ||
             value() == this->instruction()->initial_value().get<uint64_t>()));
  }

  value_type value() const { return storage_->get<value_type>(); }
  const char *value_name() const {
    // assert(instruction() && value() < instruction()->num_elements_);
    return instruction()->element_name(value());
  }

  instruction_cptr instruction() const {
    return static_cast<instruction_cptr>(instruction_);
  }
  bool is_boolean() const;

private:
  enum_cref &operator=(const enum_cref &);
};

inline bool operator==(const enum_cref &lhs, const enum_cref &rhs) {
  return (lhs.absent() == rhs.absent()) &&
         (lhs.absent() || lhs.value() == rhs.value());
}

inline bool operator!=(const enum_cref &lhs, const enum_cref &rhs) {
  return !(lhs == rhs);
}
class fast_istream;

class enum_mref : public make_field_mref<enum_cref> {
  typedef make_field_mref<enum_cref> base_type;

public:
  enum_mref() {}
  enum_mref(mfast::allocator *alloc, value_storage *storage,
            instruction_cptr instruction)
      : base_type(alloc, storage, instruction) {}

  enum_mref(const enum_mref &) = default;

  void as(const enum_cref &cref) const {
    if (cref.absent()) {
      this->omit();
    } else {
      as(cref.value());
    }
  }

  void as(value_type v) const {
    this->storage()->present(1);
    this->storage()->set<value_type>(v);
  }

  void to_initial_value() const {
    *this->storage() = this->instruction()->initial_value();
  }
  value_type value() const { return this->storage()->get<value_type>(); }
};

template <> struct mref_of<enum_cref> { typedef enum_mref type; };

template <typename Tag>
class enum_field_instruction_ex : public enum_field_instruction {
public:
  enum_field_instruction_ex(
      operator_enum_t operator_id, presence_enum_t optional, uint32_t id,
      const char *name, const char *ns, const op_context_t *context,
      int_value_storage<uint64_t> initial_value, const char **element_names,
      const uint64_t *element_values, uint64_t num_elements,
      const enum_field_instruction *ref, const char *cpp_ns,
      instruction_tag tag = instruction_tag())
      : enum_field_instruction(operator_id, optional, id, name, ns, context,
                               initial_value, element_names, element_values,
                               num_elements, ref, cpp_ns, tag) {}
};

template <typename Derived, typename EnumClassType>
class enum_cref_ex : public enum_cref {
public:
  typedef typename EnumClassType::element element_type;
  typedef typename EnumClassType::instruction_type instruction_type;
  typedef const instruction_type *intruction_cptr;

  enum_cref_ex(const value_storage *storage,
               const enum_field_instruction *instruction)
      : enum_cref(storage, instruction) {}

  enum_cref_ex(const field_cref &other) : enum_cref(other) {}
  bool operator==(const Derived &v) const { return this->value() == v.value(); }
  bool operator==(element_type v) const { return this->value() == v; }
  bool operator!=(const Derived &v) const { return !(*this == v); }
  bool operator!=(element_type v) const { return !(*this == v); }
  element_type value() const {
    return static_cast<element_type>(enum_cref::value());
  }
  intruction_cptr instruction() const {
    return static_cast<intruction_cptr>(instruction_);
  }
};

template <typename Derived, typename CRefType>
class enum_mref_ex : public make_field_mref<CRefType> {
  typedef make_field_mref<CRefType> base_type;

public:
  typedef CRefType cref_type;
  typedef typename CRefType::element_type element_type;

  enum_mref_ex(mfast::allocator *alloc, value_storage *storage,
               const enum_field_instruction *instruction)
      : base_type(alloc, storage, instruction) {}

  enum_mref_ex(const field_mref_base &other) : base_type(other) {}
  operator enum_mref() const {
    return reinterpret_cast<const enum_mref &>(*this);
  }
  void as(const cref_type &ref) const {
    if (ref.absent()) {
      this->omit();
    } else {
      as(ref.value());
    }
  }

  void as(element_type v) const {
    this->storage()->present(1);
    this->storage()->template set<uint64_t>(v);
  }

  void to_initial_value() const {
    *this->storage() = this->instruction()->initial_value();
  }
};
}
