// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.

#include "int_instructions.h"

namespace mfast {

integer_field_instruction_base::integer_field_instruction_base(
    operator_enum_t operator_id, int field_type, presence_enum_t optional,
    uint32_t id, const char *name, const char *ns, const op_context_t *context,
    const value_storage &initial_storage, instruction_tag tag)
    : field_instruction(operator_id, field_type, optional, id, name, ns, tag),
      op_context_(context), initial_value_(initial_storage),
      prev_value_(&prev_storage_),
      initial_or_default_value_(initial_storage.is_empty() ? &default_value_
                                                           : &initial_value_) {
  has_initial_value_ = !initial_storage.is_empty();
}

integer_field_instruction_base::integer_field_instruction_base(
    const integer_field_instruction_base &other)
    : field_instruction(other), op_context_(other.op_context_),
      initial_value_(other.initial_value_), prev_value_(&prev_storage_),
      initial_or_default_value_(initial_value_.is_empty() ? &default_value_
                                                          : &initial_value_) {}

void integer_field_instruction_base::construct_value(
    value_storage &storage, allocator * /* alloc */) const {
  storage.of_uint64.content_ = initial_value_.of_uint64.content_;
  storage.of_uint64.defined_bit_ = 1;
  storage.of_uint64.present_ = !optional();
}

const value_storage integer_field_instruction_base::default_value_(1);

} /* mfast */
