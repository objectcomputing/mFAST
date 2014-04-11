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

#include "boolean_ref.h"

namespace mfast
{

  static const char* boolean_elements[] = {"false","true"};

  const boolean::instruction_type*
  boolean::instruction()
  {
    static const boolean_cref::instruction_type
      the_instruction(operator_none,presence_mandatory,0,"boolean","",0,0,boolean_elements,0, 2,0,"mfast");

    return &the_instruction;
  }

  bool enum_cref::is_boolean() const
  {
    return this->instruction()->elements() == boolean_elements;
  }

} /* mfast */
