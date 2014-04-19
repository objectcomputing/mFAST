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
#ifndef TEMPLATE_REGISTRY_IMPL_H_1NKEVRXF
#define TEMPLATE_REGISTRY_IMPL_H_1NKEVRXF

#include <map>
#include <string>
#include "dynamic_templates_description.h"

namespace mfast {
  namespace xml_parser {

    struct template_registry_impl
    {
      typedef std::map<std::string, const field_instruction*> map_type;
      map_type types_;
      arena_allocator alloc_;


      std::string get_key(const char* ns, const char* name) const
      {
        return std::string(ns) + "||" + name;
      }

      const field_instruction*
      find(const char* ns, const char* name) const
      {
        map_type::const_iterator itr = types_.find(get_key(ns, name));
        if (itr != types_.end()) {
          return itr->second;
        }
        return 0;
      }

      void add(const char* ns, const field_instruction* inst)
      {
        types_[get_key(ns, inst->name())] = inst;
      }

    };

  }
}
#endif /* end of include guard: TEMPLATE_REGISTRY_IMPL_H_1NKEVRXF */
