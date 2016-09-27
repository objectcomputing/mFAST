// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.
#pragma once

#include "tinyxml2.h"
#include "../arena_allocator.h"

namespace mfast {
namespace xml_parser {
using namespace tinyxml2;
inline const char *get_optional_attr(const XMLElement &element,
                                     const char *attr_name,
                                     const char *default_value) {
  const XMLAttribute *attr = element.FindAttribute(attr_name);
  if (attr == nullptr) {
    return default_value;
  }
  return attr->Value();
}

inline const char *string_dup(const char *str, arena_allocator &alloc) {
  if (str == nullptr || str[0] == '\x0')
    return "";
  std::size_t len = std::strlen(str) + 1;
  return reinterpret_cast<const char *>(
      std::memcpy(alloc.allocate(len), str, len));
  ;
}

} /* xml_parser */

} /* mfast */
