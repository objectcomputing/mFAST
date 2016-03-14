// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.

#include "../field_instructions.h"

namespace mfast {
void decimal_field_instruction::construct_value(value_storage &storage,
                                                allocator *) const {
  storage = initial_value_;
  storage.of_decimal.present_ = !optional();
}

void decimal_field_instruction::copy_construct_value(const value_storage &src,
                                                     value_storage &dest,
                                                     allocator * /* alloc */,
                                                     value_storage *) const {
  dest.of_decimal.present_ = src.of_decimal.present_;
  dest.of_decimal.mantissa_ = src.of_decimal.mantissa_;
  dest.of_decimal.exponent_ = src.of_decimal.exponent_;
}

decimal_field_instruction *
decimal_field_instruction::clone(arena_allocator &alloc) const {
  return new (alloc) decimal_field_instruction(*this);
}

const decimal_field_instruction *
decimal_field_instruction::default_instruction() {
  static const decimal_field_instruction inst(operator_none, presence_mandatory,
                                              0, "", "", nullptr,
                                              decimal_value_storage());
  return &inst;
}
} /* mfast */