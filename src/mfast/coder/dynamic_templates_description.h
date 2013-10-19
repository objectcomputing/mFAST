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

#include "mfast_coder_export.h"

#include "mfast/field_instruction.h"
#include "mfast/arena_allocator.h"
#include <map>
namespace mfast
{
class MFAST_CODER_EXPORT template_registry
{
private:
  typedef std::map<std::string, template_instruction*> map_type;
  map_type templates_;
  std::string get_key(const char* ns, const char* name) const;
  arena_allocator alloc_;
public:
  template_instruction* find(const char* ns, const char* name) const;
  void add(const char* ns, template_instruction* inst);
  arena_allocator* allocator();

  static template_registry* instance();
};


class MFAST_CODER_EXPORT dynamic_templates_description
  : public templates_description
{
public:
  dynamic_templates_description(const char*        xml_content,
                                const char*        cpp_ns="",
                                template_registry* registry = template_registry::instance());
};

}

#endif /* end of include guard: TEMPLATE_LOADER_H_L6DO08PL */
