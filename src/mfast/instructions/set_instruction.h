// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.

#pragma once

#include "int_instructions.h"

namespace mfast {

  class MFAST_EXPORT set_field_instruction
    : public integer_field_instruction_base,
      public referable_instruction<set_field_instruction> {
    public:
    set_field_instruction(operator_enum_t operator_id, presence_enum_t optional,
                          uint32_t id, const char *name, const char *ns,
                          const op_context_t *context,
                          int_value_storage<uint64_t> initial_value,
                          const char **elements, uint64_t num_elements,
                          const set_field_instruction *ref, const char *cpp_ns,
                          instruction_tag tag = instruction_tag()) :
      integer_field_instruction_base(operator_id, field_type_set, optional,
                                     id, name, ns, context,
                                     initial_value.storage_, tag),
      referable_instruction<set_field_instruction>(ref, cpp_ns),
      elements_(elements), num_elements_(num_elements) {}

    set_field_instruction(const set_field_instruction &other) :
      integer_field_instruction_base(other),
      referable_instruction<set_field_instruction>(other),
      elements_(other.elements_), num_elements_(other.num_elements_) {}

    virtual void accept(field_instruction_visitor &visitor,
                        void *context) const override;
    virtual set_field_instruction *clone(arena_allocator &alloc) const override;

    uint64_t num_elements() const { return num_elements_; }
    const char **elements() const { return elements_; }
    const char **elements_;
    uint64_t num_elements_;
  };

} /* mfast */
