// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.

#pragma once

#include <cstddef>
#include <cassert>
#include "mfast/value_storage.h"
#include "mfast/mfast_export.h"
#include "mfast/arena_allocator.h"
#include "mfast/allocator.h"
#include <algorithm>
#include <iostream>

namespace mfast {
enum operator_enum_t {
  operator_none,
  operator_constant,
  operator_delta,
  operator_default,
  operator_copy,
  operator_increment,
  operator_tail,
  operators_count
};

enum presence_enum_t { presence_mandatory = 0, presence_optional = 1 };

enum field_type_enum_t {
  field_type_int32,
  field_type_uint32,
  field_type_int64,
  field_type_uint64,
  field_type_decimal,
  field_type_exponent,
  field_type_templateref,
  field_type_ascii_string, // using of_array start
  field_type_unicode_string,
  field_type_byte_vector,
  field_type_int32_vector,
  field_type_uint32_vector,
  field_type_int64_vector,
  field_type_uint64_vector,
  field_type_sequence, // using of_array end, codegen needed start
  field_type_group,
  field_type_template,
  field_type_enum
};

enum property_enum_t {
  field_has_initial_value = 2,
  field_previous_value_shared = 4
};

struct op_context_t {
  const char *key_;
  const char *ns_;
  const char *dictionary_;
};

template <typename T> struct field_type_trait;

template <> struct field_type_trait<int32_t> {
  enum { id = field_type_int32 };
};

template <> struct field_type_trait<uint32_t> {
  enum { id = field_type_uint32 };
};

template <> struct field_type_trait<int64_t> {
  enum { id = field_type_int64 };
};

template <> struct field_type_trait<uint64_t> {
  enum { id = field_type_uint64 };
};

class field_instruction_visitor;

class instruction_tag {
public:
  inline instruction_tag(uint64_t v = 0) : uint64_value_(v) {}
  uint64_t to_uint64() const { return uint64_value_; }

private:
  uint64_t uint64_value_;
};

inline std::ostream &operator<<(std::ostream &os, const instruction_tag &tag) {
  os << "mfast::instruction_tag(" << tag.to_uint64() << ")";
  return os;
}

class MFAST_EXPORT field_instruction {
public:
  virtual void construct_value(value_storage &storage,
                               allocator *alloc) const = 0;
  virtual void destruct_value(value_storage &storage, allocator *alloc) const;

  /// Perform deep copy
  virtual void
  copy_construct_value(const value_storage &src, value_storage &dest,
                       allocator *alloc,
                       value_storage *fields_storage = nullptr) const;

  virtual void accept(field_instruction_visitor &, void *) const = 0;
  virtual field_instruction *clone(arena_allocator &alloc) const = 0;
  virtual std::size_t pmap_size() const;

  bool is_nullable() const { return nullable_flag_; }
  bool optional() const { return optional_flag_; }
  uint32_t id() const { return id_; }
  const char *name() const { return name_; }
  const char *ns() const { return ns_; }
  field_type_enum_t field_type() const {
    return static_cast<field_type_enum_t>(field_type_);
  }
  operator_enum_t field_operator() const {
    return static_cast<operator_enum_t>(operator_id_);
  }
  bool mandatory_without_initial_value() const {
    return !optional_flag_ && !has_initial_value_;
  }
  /// @returns true if the field type is string, byteVector or sequence.
  bool is_array() const { return is_array_; }
  int properties() const {
    // used for static visitor, the first bit describe whether the field is
    // optional,
    // the second bit describe whether the filed has initial value.
    return (optional() ? 1 : 0) |
           (has_initial_value_ ? field_has_initial_value : 0) |
           (previous_value_shared_ ? field_previous_value_shared : 0);
  }

  const char *field_type_name() const;
  inline field_instruction(operator_enum_t operator_id, int field_type,
                           presence_enum_t optional, uint32_t id,
                           const char *name, const char *ns,
                           instruction_tag tag);

  field_instruction(const field_instruction &other);

  void optional(bool v) {
    optional_flag_ = v;
    update_invariant();
  }

  void id(uint32_t v) { id_ = v; }
  void name(const char *v) { name_ = v; }
  void ns(const char *v) { ns_ = v; }
  void field_operator(operator_enum_t v) {
    operator_id_ = v;
    update_invariant();
  }

  const instruction_tag &tag() const { return tag_; }
  bool previous_value_shared() const { return previous_value_shared_; }
  void previous_value_shared(bool shared) { previous_value_shared_ = shared; }

protected:
  virtual void update_invariant() {
    nullable_flag_ = optional_flag_ && (operator_id_ != operator_constant);
    has_pmap_bit_ = operator_id_ > operator_delta ||
                    ((operator_id_ == operator_constant) && optional_flag_);
  }

  // uint16_t field_index_;
  uint16_t operator_id_ : 3;
  uint16_t is_array_ : 1;
  uint16_t optional_flag_ : 1;
  uint16_t nullable_flag_ : 1;
  uint16_t has_pmap_bit_ : 1;
  uint16_t has_initial_value_ : 1;
  uint16_t field_type_ : 7;
  uint16_t previous_value_shared_ : 1;
  uint32_t id_;
  const char *name_;
  const char *ns_;
  instruction_tag tag_;
};

template <typename T> class referable_instruction {
public:
  referable_instruction(const T *ref_instruction, const char *cpp_ns)
      : ref_instruction_(ref_instruction), cpp_ns_(cpp_ns) {}

  const field_instruction *ref_instruction() const { return ref_instruction_; }
  void ref_instruction(const T *r) { ref_instruction_ = r; }
  const char *cpp_ns() const { return cpp_ns_; }

private:
  const T *ref_instruction_;
  const char *cpp_ns_;
};

inline field_instruction::field_instruction(operator_enum_t operator_id,
                                            int field_type,
                                            presence_enum_t optional,
                                            uint32_t id, const char *name,
                                            const char *ns, instruction_tag tag)
    : operator_id_(operator_id),
      is_array_(field_type >= field_type_ascii_string &&
                field_type <= field_type_sequence),
      optional_flag_(optional),
      nullable_flag_(optional && (operator_id != operator_constant)),
      has_pmap_bit_(operator_id > operator_delta ||
                    ((operator_id == operator_constant) && optional)),
      has_initial_value_(false), field_type_(field_type),
      previous_value_shared_(false), id_(id), name_(name), ns_(ns),
      tag_(std::move(tag)) {}
}
