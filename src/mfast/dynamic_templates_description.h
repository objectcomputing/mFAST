// Copyright (c) 2013, Huang-Ming Huang,  Object Computing, Inc.
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
#ifndef TEMPLATE_LOADER_H_L6DO08PL
#define TEMPLATE_LOADER_H_L6DO08PL


#include "field_instruction.h"
#include "arena_allocator.h"
namespace mfast
{
class dynamic_templates_description
  : public templates_description
{
  public:
    dynamic_templates_description(const char* xml_content);

  private:
    arena_allocator alloc_;
};

}

#endif /* end of include guard: TEMPLATE_LOADER_H_L6DO08PL */
