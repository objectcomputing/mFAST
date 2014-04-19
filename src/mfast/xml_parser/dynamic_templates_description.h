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
#ifndef TEMPLATE_LOADER_H_L6DO08PL
#define TEMPLATE_LOADER_H_L6DO08PL

#include "mfast_xml_parser_export.h"
#include "mfast/field_instructions.h"
#include "mfast/arena_allocator.h"
#include "mfast/view_iterator.h"
#include <deque>
namespace mfast
{
  namespace xml_parser {
    struct template_registry_impl;
    class templates_builder;
  }

  struct template_registry_impl;
  class MFAST_XML_PARSER_EXPORT template_registry
  {
  private:
    xml_parser::template_registry_impl* impl_;
    friend class xml_parser::templates_builder;

  public:
    template_registry();
    ~template_registry();

    arena_allocator* alloc();

    static template_registry* instance();
  };

#ifdef _MSC_VER
  template class MFAST_XML_PARSER_EXPORT std::deque<const field_instruction*>;
  template class MFAST_XML_PARSER_EXPORT std::deque<aggregate_view_info>;
#endif

  class MFAST_XML_PARSER_EXPORT dynamic_templates_description
    : public templates_description
  {
  public:
    dynamic_templates_description(const char*        xml_content,
                                  const char*        cpp_ns="",
                                  template_registry* registry = template_registry::instance());

    const std::deque<const field_instruction*>& defined_type_instructions() const;
    const std::deque<aggregate_view_info>& view_infos() const
    {
      return view_infos_;
    }

  private:
    friend class xml_parser::templates_builder;
    std::deque<const field_instruction*> defined_type_instructions_;
    std::deque<aggregate_view_info> view_infos_;
  };

}

#endif /* end of include guard: TEMPLATE_LOADER_H_L6DO08PL */
