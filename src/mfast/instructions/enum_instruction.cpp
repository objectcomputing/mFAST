// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.

#include "enum_instruction.h"

namespace mfast {

enum_field_instruction *
enum_field_instruction::clone(arena_allocator &alloc) const {
  return new (alloc) enum_field_instruction(*this);
}

} /* mfast */