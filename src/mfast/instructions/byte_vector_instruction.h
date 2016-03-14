// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.

#pragma once

#include "string_instructions.h"

namespace mfast {
class MFAST_EXPORT byte_vector_field_instruction
    : public unicode_field_instruction {
public:
  byte_vector_field_instruction(operator_enum_t operator_id,
                                presence_enum_t optional, uint32_t id,
                                const char *name, const char *ns,
                                const op_context_t *context,
                                string_value_storage initial_value,
                                uint32_t length_id, const char *length_name,
                                const char *length_ns,
                                instruction_tag tag = instruction_tag())
      : unicode_field_instruction(operator_id, optional, id, name, ns, context,
                                  initial_value, length_id, length_name,
                                  length_ns, tag, field_type_byte_vector) {}

  byte_vector_field_instruction(const byte_vector_field_instruction &other)
      : unicode_field_instruction(other) {}

  virtual void accept(field_instruction_visitor &visitor,
                      void *context) const override;
  virtual byte_vector_field_instruction *
  clone(arena_allocator &alloc) const override;

  static std::ptrdiff_t hex2binary(const char *src, unsigned char *target);

  static const byte_vector_field_instruction *default_instruction();
};

} /* mfast */
