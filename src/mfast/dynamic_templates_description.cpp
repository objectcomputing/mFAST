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
#include "dynamic_templates_description.h"
#include "../fastxml/FastXMLVisitor.h"
#include <deque>
#include <boost/assign/list_of.hpp> // for 'map_list_of()'
#include <boost/assert.hpp>
#include <boost/lexical_cast.hpp>
#include <map>
#include <set>

#include "exceptions.h"
using namespace std;
using namespace boost::assign; // bring 'map_list_of()' into scope

namespace mfast
{

// parse a decimal string representation (like 120 or 0.12) and store
// it into a nullable_decimal object.
std::istream &
operator>>(std::istream &source, nullable_decimal& result)
{
  std::string decimal_string;
  source >> decimal_string;

  std::size_t float_pos = decimal_string.find_first_of('.');
  std::size_t nonzero_pos = decimal_string.find_last_not_of(".0");

  if (float_pos == std::string::npos || nonzero_pos < float_pos) {
    // this is an integer
    if (float_pos != std::string::npos) {
      decimal_string = decimal_string.substr(0, float_pos);
      nonzero_pos = decimal_string.find_last_not_of('0');
    }
    if (nonzero_pos == std::string::npos)
      nonzero_pos = 0;
    result.set(boost::lexical_cast<int64_t>(decimal_string.substr(0, nonzero_pos+1)),
               decimal_string.size()-1 - nonzero_pos);
  }
  else {
    decimal_string = decimal_string.substr(0, nonzero_pos+1);
    result.set( boost::lexical_cast<int64_t>(decimal_string.erase(float_pos, 1)), float_pos - decimal_string.size() +1);
  }
  return source;
}

std::map<std::string,operator_enum_t> operator_map =
  map_list_of("none",operator_none)
    ("constant",operator_constant)
    ("delta",operator_delta)
    ("default",operator_default)
    ("copy",operator_copy)
    ("increment", operator_increment)
    ("tail",operator_tail);

struct cstr_compare
{
  bool operator()(const char* lhs, const char* rhs) const
  {
    return strcmp(lhs, rhs) < 0;
  }

};

class templates_loader
  : public FastXMLVisitor
{
  typedef std::deque<field_instruction*> instruction_list_t;
  std::deque<instruction_list_t> stack_;
  typedef const field_instruction* instruction_cptr;
  templates_description* definition_;
  arena_allocator* alloc_;

  public:
    templates_loader(templates_description* definition, arena_allocator* alloc)
      : definition_(definition)
      , alloc_(alloc)
    {
    }

    instruction_list_t& current()
    {
      return stack_.back();
    }

    const char* new_string(const char* src)
    {
      if (src == 0)
        return 0;
      int len = strlen(src);
      if (len == 0)
        return "";
      char* dst = new (*alloc_) char[len+1];
      std::strcpy(dst, src);
      return dst;
    }

    instruction_cptr* current_instructions()
    {
      instruction_cptr* instructions = new (*alloc_)instruction_cptr[current().size()];
      std::copy(current().begin(), current().end(), instructions);
      return instructions;
    }

    uint32_t get_id(const XMLElement & element)
    {
      return boost::lexical_cast<uint32_t>(get_optional_attr(element, "id", "0"));
    }

    presence_enum_t get_presence(const XMLElement & element)
    {
      const char* presence_str = get_optional_attr(element, "presence", "");
      if (strcmp(presence_str, "optional") ==0)
        return presence_optional;
      return presence_mandatory;
    }

    const char* get_typeRef_name(const XMLElement & element)
    {
      const XMLElement* typeRefElem = element.FirstChildElement("typeRef");
      if (typeRefElem) {
        return new_string(get_optional_attr(*typeRefElem, "name", ""));
      }
      return "";
    }

    const char* get_typeRef_ns(const XMLElement & element)
    {
      const XMLElement* typeRefElem = element.FirstChildElement("typeRef");
      if (typeRefElem) {
        return new_string(get_optional_attr(*typeRefElem, "ns", ""));
      }
      return "";
    }

    const char* get_ns(const XMLElement & element)
    {
      return new_string(get_optional_attr(element, "ns", ""));
    }

    const char* get_dictionary(const XMLElement & element)
    {
      return new_string(get_optional_attr(element, "dictionary", ""));
    }

    const char* get_templateNs(const XMLElement & element)
    {
      return new_string(get_optional_attr(element, "templateNs", ""));
    }

    const XMLElement* field_op_element(const XMLElement & element)
    {
      static const char* field_op_names[] = {
        "constant","default","copy","increment","delta","tail"
      };

      static std::set<const char*, cstr_compare> field_op_set (field_op_names, field_op_names + 6);

      for (const XMLElement* child = element.FirstChildElement(); child != 0; child = child->NextSiblingElement())
      {
        if (field_op_set.count(child->Name())) {
          return child;
        }
      }
      return 0;
    }

    bool get_field_attributes(const XMLElement & element,
                              const std::string& /* name_attr */,
                              operator_enum_t&   fieldOp,
                              op_context_t*&     opContext,
                              std::string&       initialValue)
    {
      fieldOp = operator_none;
      opContext = 0;
      initialValue = "";

      const XMLElement* fieldOpElem = field_op_element(element);

      if (fieldOpElem) {
        static std::map<std::string,operator_enum_t> operator_map =
          map_list_of("none",operator_none)
            ("constant",operator_constant)
            ("delta",operator_delta)
            ("default",operator_default)
            ("copy",operator_copy)
            ("increment", operator_increment)
            ("tail",operator_tail);

        std::map<std::string,operator_enum_t>::iterator itr = operator_map.find(fieldOpElem->Name());
        if (itr == operator_map.end())
        {
          BOOST_THROW_EXCEPTION(fast_static_error("S1") << reason_info(std::string("Invalid field operator ") + fieldOpElem->Name()));
        }
        fieldOp = operator_map[fieldOpElem->Name()];

        std::string opContext_key = get_optional_attr(*fieldOpElem, "key", "");
        if (!opContext_key.empty()) {
          opContext = new (*alloc_)op_context_t;
          opContext->key_ = new_string(opContext_key.c_str());
          opContext->ns_ = new_string(get_optional_attr(*fieldOpElem, "ns", ""));
          opContext->dictionary_ = new_string(get_optional_attr(*fieldOpElem, "dictionary", ""));
        }
        initialValue = get_optional_attr(*fieldOpElem, "value", "");
        return true;
      }
      return false;
    }

  public:
    virtual bool VisitEnterTemplates(const XMLElement & /* element */)
    {
      stack_.push_back(instruction_list_t());
      return true;
    }

    virtual bool VisitExitTemplates(const XMLElement & element,
                                    std::size_t /* numFields */)
    {
      definition_->ns_ = get_ns(element);
      definition_->template_ns_ = get_templateNs(element);
      definition_->dictionary_ = get_dictionary(element);
      definition_->instructions_ = reinterpret_cast<template_instruction const**>(current_instructions());
      definition_->instructions_count_ = current().size();
      return true;
    }

    virtual bool VisitEnterTemplate(const XMLElement & /* element */,
                                    const std::string& /* name_attr */,
                                    std::size_t /* index */)
    {
      stack_.push_back(instruction_list_t());
      return true;
    }

    virtual bool VisitExitTemplate(const XMLElement & element,
                                   const std::string& name_attr,
                                   std::size_t /* numFields */,
                                   std::size_t /* index */)
    {
      bool reset = false;
      const XMLAttribute* reset_attr = element.FindAttribute("scp:reset");
      if (reset_attr == 0)
        reset_attr = element.FindAttribute("reset");

      if (reset_attr) {
        if (strcmp(reset_attr->Value(), "true") == 0 || strcmp(reset_attr->Value(), "yes") == 0)
          reset = true;
      }

      template_instruction* instruction = new (*alloc_)template_instruction(
        get_id(element),
        new_string(name_attr.c_str()),
        get_ns(element),
        get_templateNs(element),
        get_dictionary(element),
        current_instructions(),
        current().size(),
        reset,
        get_typeRef_name(element),
        get_typeRef_ns(element)
        );
      stack_.pop_back();
      current().push_back(instruction);
      return true;
    }

    virtual bool VisitTemplateRef(const XMLElement & element,
                                  const std::string& name_attr,
                                  std::size_t        index)
    {
      templateref_instruction* instruction;

      if (name_attr.size()) {
        instruction = new (*alloc_)templateref_instruction(
          static_cast<uint16_t>(index),
          new_string(name_attr.c_str()),
          get_templateNs(element));
      }
      else {
        instruction = new (*alloc_)templateref_instruction(
          static_cast<uint16_t>(index));
      }

      current().push_back(instruction);
      return true;
    }

    virtual bool VisitEnterGroup(const XMLElement & /* element */,
                                 const std::string& /* name_attr */,
                                 std::size_t /* index */)
    {
      stack_.push_back(instruction_list_t());
      return true;
    }

    virtual bool VisitExitGroup(const XMLElement & element,
                                const std::string& name_attr,
                                std::size_t /* numFields */,
                                std::size_t        index)
    {
      group_field_instruction* instruction = new (*alloc_)group_field_instruction (
        static_cast<uint16_t>(index),
        get_presence(element),
        get_id(element),
        new_string(name_attr.c_str()),
        get_ns(element),
        get_dictionary(element),
        current_instructions(),
        current().size(),
        get_typeRef_name(element),
        get_typeRef_ns(element)
        );

      stack_.pop_back();
      current().push_back(instruction);
      return true;
    }

    virtual bool VisitEnterSequence(const XMLElement & /* element */,
                                    const std::string& /* name_attr */,
                                    std::size_t /* index */)
    {
      stack_.push_back(instruction_list_t());
      return true;
    }

    virtual bool VisitExitSequence(const XMLElement & element,
                                   const std::string& name_attr,
                                   std::size_t /* numFields */,
                                   std::size_t        index)
    {
      const XMLElement* length_element = element.FirstChildElement("length");
      uint32_field_instruction* length_instruction = 0;

      operator_enum_t fieldOp= operator_none;
      uint32_t id = 0;
      op_context_t* opContext=0;
      std::string initial_value_str;
      std::string length_name = name_attr + "___length___";
      std::string ns;
      nullable<uint32_t> initial_value;

      if (length_element) {
        id = get_id(*length_element);
        get_field_attributes(*length_element, name_attr, fieldOp, opContext, initial_value_str );

        length_name = get_optional_attr(*length_element, "name", "");
        ns = get_ns(*length_element);
        if (initial_value_str.size()) {
          initial_value.set(boost::lexical_cast<uint32_t>(initial_value_str));
        }
      }

      length_instruction = new (*alloc_)uint32_field_instruction(
        0,
        fieldOp,
        get_presence(element),
        id,
        new_string(length_name.c_str()),
        new_string(ns.c_str()),
        opContext,
        initial_value);

      sequence_field_instruction* instruction = new (*alloc_)sequence_field_instruction(
        static_cast<uint16_t>(index),
        get_presence(element),
        get_id(element),
        new_string(name_attr.c_str()),
        get_ns(element),
        get_dictionary(element),
        current_instructions(),
        current().size(),
        length_instruction,
        get_typeRef_name(element),
        get_typeRef_ns(element));

      stack_.pop_back();
      current().push_back(instruction);
      return true;
    }

    template <typename INT_TYPE>
    void gen_integer_instruction(const XMLElement & element,
                                 const std::string& name_attr,
                                 std::size_t        index)
    {
      operator_enum_t fieldOp;
      op_context_t* opContext;
      std::string initial_value_str;

      nullable<INT_TYPE> initial_value;

      get_field_attributes(element, name_attr, fieldOp, opContext, initial_value_str );

      if (initial_value_str.size()) {
        initial_value.set(boost::lexical_cast<INT_TYPE>(initial_value_str));
      }

      typedef typename instruction_trait<INT_TYPE>::type intruction_t;
      intruction_t* instruction= new (*alloc_)intruction_t  (
        static_cast<uint16_t>(index),
        fieldOp,
        get_presence(element),
        get_id(element),
        new_string(name_attr.c_str()),
        get_ns(element),
        opContext,
        initial_value
        );

      current().push_back(instruction);
    }

    virtual bool VisitInteger(const XMLElement & element,
                              int                integer_bits,
                              const std::string& name_attr,
                              std::size_t        index)
    {
      bool is_unsigned = element.Name()[0] == 'u';
      if (is_unsigned) {
        if (integer_bits == 64)
          gen_integer_instruction<uint64_t>(element, name_attr, index);
        else
          gen_integer_instruction<uint32_t>(element, name_attr, index);
      }
      else {
        if (integer_bits == 64)
          gen_integer_instruction<int64_t>(element, name_attr, index);
        else
          gen_integer_instruction<int32_t>(element, name_attr, index);
      }
      return true;
    }

    virtual bool VisitDecimal(const XMLElement & element,
                              const std::string& name_attr,
                              std::size_t        index)
    {
      decimal_field_instruction* instruction;
      const XMLElement* mantissa_element = element.FirstChildElement("mantissa");
      const XMLElement* exponent_element = element.FirstChildElement("exponent");

      if (mantissa_element || exponent_element) {

        mantissa_field_instruction* mantissa_instruction =0;

        if (mantissa_element)
        {
          operator_enum_t mantissa_fieldOp;
          op_context_t* mantissa_opContext;
          std::string mantissa_initial_value_str;

          get_field_attributes(*mantissa_element,
                               name_attr + "_mantissa",
                               mantissa_fieldOp,
                               mantissa_opContext,
                               mantissa_initial_value_str);


          nullable<int64_t> mantissa_initial_value;

          if (mantissa_initial_value_str.size()) {
            mantissa_initial_value.set(boost::lexical_cast<int64_t>(mantissa_initial_value_str.c_str()));
          }

          mantissa_instruction = new (*alloc_)mantissa_field_instruction(
            mantissa_fieldOp,
            mantissa_opContext,
            mantissa_initial_value
            );
        }

        operator_enum_t exponent_fieldOp = operator_none;
        op_context_t* exponent_opContext = 0;
        nullable<int8_t> exponent_initial_value;

        if (exponent_element) {

          std::string exponent_initial_value_str;
          get_field_attributes(*exponent_element, name_attr + "_exponent",
                               exponent_fieldOp,
                               exponent_opContext,
                               exponent_initial_value_str);

          if (exponent_initial_value_str.size()) {
            exponent_initial_value.set(boost::lexical_cast<int8_t>(exponent_initial_value_str.c_str()));
          }
        }

        instruction = new (*alloc_)decimal_field_instruction(
          static_cast<uint16_t>(index),
          exponent_fieldOp,
          get_presence(element),
          get_id(element),
          new_string(name_attr.c_str()),
          get_ns(element),
          exponent_opContext,
          mantissa_instruction,
          exponent_initial_value);
      }
      else {

        operator_enum_t fieldOp;
        op_context_t* opContext;
        std::string initial_value_str;
        nullable_decimal initial_value;

        get_field_attributes(element, name_attr, fieldOp, opContext, initial_value_str);

        if (initial_value_str.size()) {
          initial_value = boost::lexical_cast<nullable_decimal>(initial_value_str.c_str());
        }

        instruction = new (*alloc_)decimal_field_instruction(
          static_cast<uint16_t>(index),
          fieldOp,
          get_presence(element),
          get_id(element),
          new_string(name_attr.c_str()),
          get_ns(element),
          opContext,
          initial_value);
      }
      current().push_back(instruction);
      return true;
    }

    template <typename InstructionType>
    void gen_string_instruction(const XMLElement & element,
                                const std::string& name_attr,
                                std::size_t        index)
    {
      operator_enum_t fieldOp;
      op_context_t* opContext;
      std::string initial_value_str;

      get_field_attributes(element, name_attr, fieldOp, opContext, initial_value_str );


      InstructionType* instruction= new (*alloc_)InstructionType  (
        static_cast<uint16_t>(index),
        fieldOp,
        get_presence(element),
        get_id(element),
        new_string(name_attr.c_str()),
        get_ns(element),
        opContext,
        new_string(initial_value_str.c_str()),
        initial_value_str.size()
        );

      current().push_back(instruction);
    }

    virtual bool VisitString(const XMLElement & element,
                             const std::string& name_attr,
                             std::size_t        index)
    {
      const char* charset =  get_optional_attr(element, "charset", "ascii");
      if (strcmp(charset, "ascii") == 0) {
        gen_string_instruction<ascii_field_instruction>( element, name_attr, index);
      }
      else {
        gen_string_instruction<unicode_field_instruction>(element, name_attr, index);
      }
      return true;
    }

    ptrdiff_t hex2binary(const char* src, unsigned char* target)
    {
      unsigned char* dest = target;
      char c = 0;
      int shift_bits = 4;

      for (; *src != '\0'; ++src) {

        char tmp =0;

        if (*src >= '0' && *src <= '9') {
          tmp = (*src - '0');
        }
        else if (*src >= 'A' && *src <= 'F') {
          tmp = (*src - 'A') + '\x0A';
        }
        else if (*src >= 'a' && *src <= 'f') {
          tmp = (*src - 'a') + '\x0a';
        }
        else if (*src == ' ')
          continue;
        else
          return -1;

        c |= (tmp << shift_bits);

        if (shift_bits == 0) {
          *dest++ = c;
          c = 0;
          shift_bits = 4;
        }
        else
          shift_bits = 0;
      }

      if (shift_bits == 0)
        return -1;

      return dest - target;
    }

    virtual bool VisitByteVector(const XMLElement & element,
                                 const std::string& name_attr,
                                 std::size_t        index)
    {
      operator_enum_t fieldOp;
      op_context_t* opContext;
      std::string initial_value_str;

      get_field_attributes(element, name_attr, fieldOp, opContext, initial_value_str );

      uint32_t length_id =0;
      const char* length_name=0;
      const char* length_ns=0;
      const XMLElement* length_element = element.FirstChildElement("length");
      if (length_element) {
        length_id = get_id(*length_element);
        length_name = new_string(get_optional_attr(*length_element, "name", ""));
        length_ns = get_ns(*length_element);
      }

      unsigned char* initial_value=0;
      int32_t initial_value_len=0;

      if (initial_value_str.size()) {
        initial_value = static_cast<unsigned char*>(alloc_->allocate(initial_value_str.size()/2+1));
        initial_value_len = hex2binary(initial_value_str.c_str(), initial_value);
        if (initial_value_len == -1) {
          BOOST_THROW_EXCEPTION(fast_dynamic_error("D11") << reason_info(std::string("Invalid byteVector initial value: ") +  initial_value_str ) );
        }
      }

      byte_vector_field_instruction* instruction = new (*alloc_)byte_vector_field_instruction(
        static_cast<uint16_t>(index),
        fieldOp,
        get_presence(element),
        get_id(element),
        new_string(name_attr.c_str()),
        get_ns(element),
        opContext,
        initial_value,
        initial_value_len,
        length_id,
        length_name,
        length_ns
        );

      current().push_back(instruction);
      return true;
    }

};


dynamic_templates_description::dynamic_templates_description(const char* xml_content)
{
  tinyxml2::XMLDocument document;
  if (document.Parse(xml_content) == 0) {
    templates_loader loader(this, &this->alloc_);
    document.Accept(&loader);
  }
  else {
    BOOST_THROW_EXCEPTION(fast_static_error("S1"));
  }
}

}
