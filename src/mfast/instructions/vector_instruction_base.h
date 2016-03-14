// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.

#pragma once

#include "field_instruction.h"

namespace mfast {
class MFAST_EXPORT vector_field_instruction_base : public field_instruction {
public:
  vector_field_instruction_base(operator_enum_t operator_id,
                                field_type_enum_t field_type,
                                presence_enum_t optional, uint32_t id,
                                const char *name, const char *ns,
                                std::size_t element_size, instruction_tag tag)
      : field_instruction(operator_id, field_type, optional, id, name, ns, tag),
        element_size_(element_size) {}

  vector_field_instruction_base(const vector_field_instruction_base &other)
      : field_instruction(other), element_size_(other.element_size_) {}

  virtual void construct_value(value_storage &storage,
                               allocator *alloc) const override;
  virtual void destruct_value(value_storage &storage,
                              allocator *alloc) const override;

  // perform deep copy
  virtual void
  copy_construct_value(const value_storage &src, value_storage &dest,
                       allocator *alloc,
                       value_storage *fields_storage = nullptr) const override;

protected:
  std::size_t element_size_;
};

} /* mfast */
