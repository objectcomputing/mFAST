// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.

#pragma once

#include "vector_instruction_base.h"

namespace mfast {
class MFAST_EXPORT ascii_field_instruction
    : public vector_field_instruction_base {
public:
  ascii_field_instruction(
      operator_enum_t operator_id, presence_enum_t optional, uint32_t id,
      const char *name, const char *ns, const op_context_t *context,
      string_value_storage initial_value,
      instruction_tag tag = instruction_tag(),
      field_type_enum_t field_type = field_type_ascii_string);

  ascii_field_instruction(const ascii_field_instruction &other);

  virtual void construct_value(value_storage &storage,
                               allocator *alloc) const override;
  // perform deep copy
  virtual void
  copy_construct_value(const value_storage &src, value_storage &dest,
                       allocator *alloc,
                       value_storage *fields_storage = nullptr) const override;

  virtual void accept(field_instruction_visitor &visitor,
                      void *context) const override;
  virtual ascii_field_instruction *clone(arena_allocator &alloc) const override;

  value_storage &prev_value() { return *prev_value_; }
  const value_storage &prev_value() const { return *prev_value_; }
  const op_context_t *op_context() const { return op_context_; }
  void op_context(const op_context_t *v) { op_context_ = v; }
  const value_storage &initial_value() const { return initial_value_; }
  const value_storage &initial_or_default_value() const {
    return *initial_or_default_value_;
  }
  void initial_value(const value_storage &v);

  static const ascii_field_instruction *default_instruction();

protected:
  virtual void update_invariant() override {
    field_instruction::update_invariant();
    has_initial_value_ = !initial_value_.is_empty();
  }

  friend class dictionary_builder;
  const op_context_t *op_context_;
  value_storage initial_value_;
  value_storage *prev_value_;
  value_storage prev_storage_;
  const value_storage *initial_or_default_value_;
  static const value_storage default_value_;
};

class MFAST_EXPORT unicode_field_instruction : public ascii_field_instruction {
public:
  unicode_field_instruction(
      operator_enum_t operator_id, presence_enum_t optional, uint32_t id,
      const char *name, const char *ns, const op_context_t *context,
      string_value_storage initial_value, // = string_value_storage(),
      uint32_t length_id,                 //= 0,
      const char *length_name,            //= "",
      const char *length_ns,              //= "",
      instruction_tag tag = instruction_tag(),
      field_type_enum_t field_type = field_type_unicode_string)
      : ascii_field_instruction(operator_id, optional, id, name, ns, context,
                                initial_value, tag, field_type),
        length_id_(length_id), length_name_(length_name),
        length_ns_(length_ns) {}

  unicode_field_instruction(const unicode_field_instruction &other)
      : ascii_field_instruction(other), length_id_(other.length_id_),
        length_name_(other.length_name_), length_ns_(other.length_ns_) {}

  virtual void accept(field_instruction_visitor &visitor,
                      void *context) const override;
  virtual unicode_field_instruction *
  clone(arena_allocator &alloc) const override;

  uint32_t length_id() const { return length_id_; }
  const char *length_name() const { return length_name_; }
  const char *length_ns() const { return length_ns_; }
  void length_id(uint32_t v) { length_id_ = v; }
  void length_name(const char *v) { length_name_ = v; }
  void length_ns(const char *v) { length_ns_ = v; }
  static const unicode_field_instruction *default_instruction();

protected:
  uint32_t length_id_;
  const char *length_name_;
  const char *length_ns_;
};

} /* mfast */
