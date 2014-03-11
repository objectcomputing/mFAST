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
#ifndef XML_UTIL_H_N23DJCGA
#define XML_UTIL_H_N23DJCGA

#include "tinyxml2.h"
#include "mfast/arena_allocator.h"

namespace mfast
{
  namespace xml_parser
  {
    using namespace tinyxml2;
    inline const char* get_optional_attr(const XMLElement & element,
                                         const char*        attr_name,
                                         const char*        default_value)
    {
      const XMLAttribute* attr = element.FindAttribute(attr_name);
      if (attr == 0) {
        return default_value;
      }
      return attr->Value();
    }

    inline const char*
    string_dup(const char* str, arena_allocator&  alloc)
    {
      if (str == 0 || str[0] == '\x0')
        return "";
      std::size_t len = std::strlen(str);
      char* result = static_cast<char*>( alloc.allocate(len+1) );
      std::strcpy(result, str);
      return result;
    }

  } /* xml_parser */

} /* mfast */


#endif /* end of include guard: XML_UTIL_H_N23DJCGA */
