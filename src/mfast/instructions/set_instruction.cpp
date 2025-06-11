// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.

#include "set_instruction.h"

namespace mfast {

set_field_instruction *
set_field_instruction::clone(arena_allocator &alloc) const {
  return new (alloc) set_field_instruction(*this);
}

} /* mfast */
