// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.

#include <cstring>
#include "field_instruction.h"

namespace mfast {

field_instruction::field_instruction(const field_instruction &other)
    : operator_id_(other.operator_id_), is_array_(other.is_array_),
      optional_flag_(other.optional_flag_),
      nullable_flag_(other.nullable_flag_), has_pmap_bit_(other.has_pmap_bit_),
      has_initial_value_(other.has_initial_value_),
      field_type_(other.field_type_),
      previous_value_shared_(other.previous_value_shared_), id_(other.id_),
      name_(other.name_), ns_(other.ns_), tag_(other.tag_) {}

void field_instruction::destruct_value(value_storage &, allocator *) const {}

void field_instruction::copy_construct_value(const value_storage &src,
                                             value_storage &dest,
                                             allocator * /* alloc */,
                                             value_storage *) const {
  dest.of_uint64.content_ = src.of_uint64.content_;
  dest.of_array.len_ = src.of_array.len_;
  dest.of_array.capacity_in_bytes_ = 0;
}

const char *field_instruction::field_type_name() const {
  static const char *names[] = {
      "field_type_int32",          "field_type_uint32",
      "field_type_int64",          "field_type_uint64",
      "field_type_decimal",        "field_type_exponent",
      "field_type_templateref",    "field_type_ascii_string",
      "field_type_unicode_string", "field_type_byte_vector",
      "field_type_group",          "field_type_sequence",
      "field_type_template",       "field_type_enum"};
  return names[this->field_type()];
}

std::size_t field_instruction::pmap_size() const { return has_pmap_bit_; }
}
