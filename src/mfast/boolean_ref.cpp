// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.
#include "boolean_ref.h"

namespace mfast {

static const char *boolean_elements[] = {"false", "true"};

const boolean::instruction_type *boolean::instruction() {
  static const boolean_cref::instruction_type the_instruction(
      operator_none, presence_mandatory, 0, "boolean", "", nullptr, 0,
      boolean_elements, nullptr, 2, nullptr, "mfast");

  return &the_instruction;
}

bool enum_cref::is_boolean() const {
  return this->instruction()->elements() == boolean_elements;
}

} /* mfast */
