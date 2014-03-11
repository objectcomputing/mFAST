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
#ifndef FIELD_BUILDER_H_1V10ZRK6
#define FIELD_BUILDER_H_1V10ZRK6

#include <boost/lexical_cast.hpp>
#include "field_builder_base.h"
#include "fast_xml_attributes.h"
namespace mfast
{
  namespace xml_parser
  {
    typedef std::deque<const field_instruction*> instruction_list_t;

    class field_builder
      : public fast_xml_attributes
      , public field_builder_base
      , public field_instruction_visitor
    {
    protected:
      instruction_list_t instructions_;

      const char*    field_type_name_;
      const XMLElement& element_;
      const XMLElement* content_element_;
      field_builder_base*    parent_;

    public:

      field_builder(field_builder_base* parent,
                    const XMLElement&   element);


      // used for build define element
      field_builder(field_builder_base* parent,
                    const XMLElement&   element,
                    const char*         type_name);

      void build();


      const char* resolve_field_type(const XMLElement& element);

      const field_instruction* find_prototype(const char* type_name);

      const char* field_type_name() const
      {
        return field_type_name_;
      }

      virtual const char* name() const;
      virtual std::size_t num_instructions() const;
      virtual void add_instruction(const field_instruction*);
      virtual void add_template(const char* ns, template_instruction* inst);

    protected:


      template <typename Instruction>
      uint32_t get_length_id(const Instruction* inst, const fast_xml_attributes& length_attrs) const
      {
        return length_attrs.id_ ? boost::lexical_cast<uint32_t>(length_attrs.id_) : inst->length_id();
      }

      template <typename Instruction>
      const char* get_length_name(const Instruction* inst, const fast_xml_attributes& length_attrs) const
      {
        return length_attrs.name_ ? string_dup(length_attrs.name_, alloc()) : inst->length_name();
      }

      template <typename Instruction>
      const char* get_length_ns(const Instruction* inst, const fast_xml_attributes& length_attrs) const
      {
        return length_attrs.ns_ ? string_dup(length_attrs.ns_, alloc()) : inst->length_ns();
      }

      template <typename Instruction>
      const char* get_dictionary(const Instruction* inst) const
      {
        return dictionary_ ? string_dup(dictionary_, alloc()) : inst->dictionary();
      }

      virtual void visit(const int32_field_instruction*, void*);
      virtual void visit(const uint32_field_instruction*, void*);
      virtual void visit(const int64_field_instruction*, void*);
      virtual void visit(const uint64_field_instruction*, void*);
      template <typename IntType>
      void build_integer(const int_field_instruction<IntType>* inst);

      virtual void visit(const decimal_field_instruction*, void*);

      virtual void visit(const ascii_field_instruction*, void*);

      virtual void visit(const unicode_field_instruction*, void*);
      virtual void visit(const byte_vector_field_instruction*, void*);

      virtual void visit(const group_field_instruction*, void*);
      virtual void visit(const sequence_field_instruction*, void*);
      virtual void visit(const template_instruction*, void*);
      virtual void visit(const templateref_instruction*, void*);

      virtual void visit(const int32_vector_field_instruction*, void*);
      virtual void visit(const uint32_vector_field_instruction*, void*);
      virtual void visit(const int64_vector_field_instruction*, void*);
      virtual void visit(const uint64_vector_field_instruction*, void*);


      virtual void visit(const enum_field_instruction*, void*);

      instructions_view_t build_subfields();
      const group_field_instruction* get_sole_templateref();
      const uint32_field_instruction* get_length_instruction(const sequence_field_instruction* inst);

      instruction_tag parse_tag(const field_instruction* inst);
    };


  } /* xml_parser */

} /* mfast */


#endif /* end of include guard: FIELD_BUILDER_H_1V10ZRK6 */
