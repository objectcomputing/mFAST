// Copyright (c) 2013, 2014, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
//
//     mFAST is free software: you can redistribute it and/or modify
//     it under the terms of the GNU Lesser General Public License as published by
//     the Free Software Foundation, either version 3 of the License, or
//     (at your option) any later version.
//
//     mFAST is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.
//
//     You should have received a copy of the GNU Lesser General Public License
//     along with mFast.  If not, see <http://www.gnu.org/licenses/>.
//
#include "mfast/field_instructions.h"

namespace mfast {

  void decimal_field_instruction::accept(field_instruction_visitor& visitor,
                                         void*                      context) const
  {
    visitor.visit(this, context);
  }

  void enum_field_instruction::accept(field_instruction_visitor& visitor,
                                      void*                      context) const
  {
    return visitor.visit(this, context);
  }

  void ascii_field_instruction::accept(field_instruction_visitor& visitor,
                                       void*                      context) const
  {
    visitor.visit(this, context);
  }

  void unicode_field_instruction::accept(field_instruction_visitor& visitor,
                                         void*                      context) const
  {
    visitor.visit(this, context);
  }

  void byte_vector_field_instruction::accept(field_instruction_visitor& visitor,
                                             void*                      context) const
  {
    visitor.visit(this, context);
  }

  void group_field_instruction::accept(field_instruction_visitor& visitor,
                                       void*                      context) const
  {
    visitor.visit(this, context);
  }

  void sequence_field_instruction::accept(field_instruction_visitor& visitor,
                                          void*                      context) const
  {
    visitor.visit(this, context);
  }

  void template_instruction::accept(field_instruction_visitor& visitor,
                                    void*                      context) const
  {
    visitor.visit(this, context);
  }

  void templateref_instruction::accept(field_instruction_visitor& visitor,
                                       void*                      context) const
  {
    visitor.visit(this, context);
  }

}
