// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.
#include "field_instructions.h"

namespace mfast {

void decimal_field_instruction::accept(field_instruction_visitor &visitor,
                                       void *context) const {
  visitor.visit(this, context);
}

void enum_field_instruction::accept(field_instruction_visitor &visitor,
                                    void *context) const {
  return visitor.visit(this, context);
}

void ascii_field_instruction::accept(field_instruction_visitor &visitor,
                                     void *context) const {
  visitor.visit(this, context);
}

void unicode_field_instruction::accept(field_instruction_visitor &visitor,
                                       void *context) const {
  visitor.visit(this, context);
}

void byte_vector_field_instruction::accept(field_instruction_visitor &visitor,
                                           void *context) const {
  visitor.visit(this, context);
}

void group_field_instruction::accept(field_instruction_visitor &visitor,
                                     void *context) const {
  visitor.visit(this, context);
}

void sequence_field_instruction::accept(field_instruction_visitor &visitor,
                                        void *context) const {
  visitor.visit(this, context);
}

void template_instruction::accept(field_instruction_visitor &visitor,
                                  void *context) const {
  visitor.visit(this, context);
}

void templateref_instruction::accept(field_instruction_visitor &visitor,
                                     void *context) const {
  visitor.visit(this, context);
}
}
