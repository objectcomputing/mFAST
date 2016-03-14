// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.

#pragma once

#include "field_instruction.h"

namespace mfast {
class dictionary_builder;

class MFAST_EXPORT integer_field_instruction_base : public field_instruction {
public:
  integer_field_instruction_base(operator_enum_t operator_id, int field_type,
                                 presence_enum_t optional, uint32_t id,
                                 const char *name, const char *ns,
                                 const op_context_t *context,
                                 const value_storage &initial_storage,
                                 instruction_tag tag);

  integer_field_instruction_base(const integer_field_instruction_base &other);
  virtual void construct_value(value_storage &storage,
                               allocator *alloc) const override;

  value_storage &prev_value() { return *prev_value_; }
  const value_storage &prev_value() const { return *prev_value_; }
  const op_context_t *op_context() const { return op_context_; }
  void op_context(const op_context_t *v) { op_context_ = v; }
  const value_storage &initial_value() const { return initial_value_; }
  const value_storage &initial_or_default_value() const {
    return *initial_or_default_value_;
  }
  void initial_value(const value_storage &v) {
    initial_value_ = v;
    initial_or_default_value_ =
        initial_value_.is_empty() ? &default_value_ : &initial_value_;
    has_initial_value_ = !initial_value_.is_empty();
  }

protected:
  friend class dictionary_builder;
  const op_context_t *op_context_;
  value_storage initial_value_;
  value_storage *prev_value_;
  value_storage prev_storage_;
  const value_storage *initial_or_default_value_;
  static const value_storage default_value_;

  virtual void update_invariant() override {
    field_instruction::update_invariant();
    has_initial_value_ = !initial_value_.is_empty();
  }
};

template <typename T>
class int_field_instruction : public integer_field_instruction_base {
public:
  int_field_instruction(operator_enum_t operator_id, presence_enum_t optional,
                        uint32_t id, const char *name, const char *ns,
                        const op_context_t *context,
                        int_value_storage<T> initial_value,
                        instruction_tag tag = instruction_tag());

  int_field_instruction(const int_field_instruction &other);
  virtual void accept(field_instruction_visitor &visitor,
                      void *context) const override;
  virtual int_field_instruction<T> *
  clone(arena_allocator &alloc) const override;
};

template <typename T>
int_field_instruction<T>::int_field_instruction(
    operator_enum_t operator_id, presence_enum_t optional, uint32_t id,
    const char *name, const char *ns, const op_context_t *context,
    int_value_storage<T> initial_value, instruction_tag tag)
    : integer_field_instruction_base(operator_id, field_type_trait<T>::id,
                                     optional, id, name, ns, context,
                                     initial_value.storage_, tag) {}

template <typename T>
inline int_field_instruction<T>::int_field_instruction(
    const int_field_instruction &other)
    : integer_field_instruction_base(other) {}

template <typename T>
int_field_instruction<T> *
int_field_instruction<T>::clone(arena_allocator &alloc) const {
  return new (alloc) int_field_instruction<T>(*this);
}

typedef int_field_instruction<int32_t> int32_field_instruction;
typedef int_field_instruction<uint32_t> uint32_field_instruction;
typedef int_field_instruction<int64_t> int64_field_instruction;
typedef int_field_instruction<uint64_t> uint64_field_instruction;

} /* mfast */
