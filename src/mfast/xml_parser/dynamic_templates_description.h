// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.
#pragma once

#include "mfast_xml_parser_export.h"
#include "../field_instructions.h"
#include "../arena_allocator.h"
#include "../view_iterator.h"
#include <deque>
namespace mfast {
namespace xml_parser {
struct template_registry_impl;
class templates_builder;
}

struct template_registry_impl;
class MFAST_XML_PARSER_EXPORT template_registry {
private:
  xml_parser::template_registry_impl *impl_;
  friend class xml_parser::templates_builder;

public:
  template_registry();
  ~template_registry();

  arena_allocator *alloc();

  static template_registry *instance();
};

#ifdef _MSC_VER
template class MFAST_XML_PARSER_EXPORT std::deque<const field_instruction *>;
template class MFAST_XML_PARSER_EXPORT std::deque<aggregate_view_info>;
#endif

class MFAST_XML_PARSER_EXPORT dynamic_templates_description
    : public templates_description {
public:
  dynamic_templates_description(
      const char *xml_content, const char *cpp_ns = "",
      template_registry *registry = template_registry::instance());

  dynamic_templates_description(const std::string &xml_content)
      : dynamic_templates_description(xml_content.c_str()) {}

  const std::deque<const field_instruction *> &
  defined_type_instructions() const;
  const std::deque<aggregate_view_info> &view_infos() const {
    return view_infos_;
  }

private:
  friend class xml_parser::templates_builder;
  std::deque<const field_instruction *> defined_type_instructions_;
  std::deque<aggregate_view_info> view_infos_;
};
}
