// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.

#pragma once

#include "template_instruction.h"

namespace mfast {
class MFAST_EXPORT templateref_instruction : public field_instruction {
public:
  templateref_instruction(instruction_tag tag = instruction_tag());

  virtual void construct_value(value_storage &storage,
                               allocator *alloc) const override;
  virtual void destruct_value(value_storage &storage,
                              allocator *alloc) const override;

  void construct_value(value_storage &storage, allocator *alloc,
                       const template_instruction *from_inst,
                       bool construct_subfields) const;

  virtual std::size_t pmap_size() const override;

  /// Perform deep copy
  virtual void
  copy_construct_value(const value_storage &src, value_storage &dest,
                       allocator *alloc,
                       value_storage *fields_storage = nullptr) const override;

  virtual void accept(field_instruction_visitor &, void *) const override;

  virtual templateref_instruction *clone(arena_allocator &alloc) const override;

  static instructions_view_t default_instruction();
};

} /* mfast */
