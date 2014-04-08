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
#include "common/exceptions.h"
#include "dynamic_templates_description.h"
#include "common/FastXMLVisitor.h"
#include <deque>
#include <boost/assign/list_of.hpp> // for 'map_list_of()'
#include <boost/assert.hpp>
#include <boost/lexical_cast.hpp>
#include <map>
#include <set>

using namespace std;
using namespace boost::assign; // bring 'map_list_of()' into scope

namespace mfast
{
typedef std::deque<field_instruction*> instruction_list_t;

struct tag_reason;
typedef boost::error_info<tag_referenced_by,std::string> reason_info;

// parse a decimal string representation (like 120 or 0.12) and store
// it into a decimal_value_storage object.
std::istream &
operator>>(std::istream &source, decimal_value_storage& result)
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
    result = decimal_value_storage(boost::lexical_cast<int64_t>(decimal_string.substr(0, nonzero_pos+1)),
                                   decimal_string.size()-1 - nonzero_pos);
  }
  else {
    decimal_string = decimal_string.substr(0, nonzero_pos+1);
    result = decimal_value_storage( boost::lexical_cast<int64_t>(decimal_string.erase(float_pos, 1)),
                                    float_pos - decimal_string.size() +1);
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


struct template_registry_impl
{
  typedef std::map<std::string, template_instruction*> map_type;
  map_type templates_;
  arena_allocator alloc_;


  std::string get_key(const char* ns, const char* name) const
  {
    return std::string(ns) + "||" + name;
  }

  template_instruction*
  find(const char* ns, const char* name) const
  {
    // std::cerr << "template_registry::find(" << ns << "," << name << ")\n";

    map_type::const_iterator itr = templates_.find(get_key(ns, name));
    if (itr != templates_.end()) {
      return itr->second;
    }
    return 0;
  }

  void add(const char* ns, template_instruction* inst)
  {
    // std::cerr << "template_registry::add(" << ns << "," << inst->name() << ")\n";
    templates_[get_key(ns, inst->name())] = inst;
  }

};


template_registry::template_registry()
  : impl_(new template_registry_impl)
{
}

template_registry::~template_registry()
{
  delete impl_;
}

template_registry*
template_registry::instance()
{
  static template_registry inst;
  return &inst;
}

class instruction_cloner
  : public field_instruction_visitor
{
public:

  instruction_cloner(instruction_list_t& list,
                     arena_allocator&    alloc)
    : list_(list)
    , alloc_(alloc)
  {
  }

private:
  instruction_list_t& list_;
  arena_allocator&    alloc_;

  virtual void visit(const int32_field_instruction* inst, void*)
  {
    int32_field_instruction* new_inst = new (alloc_) int32_field_instruction(*inst);
    new_inst->field_index(list_.size());
    list_.push_back(new_inst);
  }

  virtual void visit(const uint32_field_instruction* inst, void*)
  {
    uint32_field_instruction* new_inst = new (alloc_) uint32_field_instruction(*inst);
    new_inst->field_index(list_.size());
    list_.push_back(new_inst);
  }

  virtual void visit(const int64_field_instruction* inst, void*)
  {
    int64_field_instruction* new_inst = new (alloc_) int64_field_instruction(*inst);
    new_inst->field_index(list_.size());
    list_.push_back(new_inst);
  }

  virtual void visit(const uint64_field_instruction* inst, void*)
  {
    uint64_field_instruction* new_inst = new (alloc_) uint64_field_instruction(*inst);
    new_inst->field_index(list_.size());
    list_.push_back(new_inst);
  }

  virtual void visit(const decimal_field_instruction* inst, void*)
  {
    decimal_field_instruction* new_inst = new (alloc_) decimal_field_instruction(*inst);
    new_inst->field_index(list_.size());
    list_.push_back(new_inst);
  }

  virtual void visit(const ascii_field_instruction* inst, void*)
  {
    ascii_field_instruction* new_inst = new (alloc_) ascii_field_instruction(*inst);
    new_inst->field_index(list_.size());
    list_.push_back(new_inst);
  }

  virtual void visit(const unicode_field_instruction* inst, void*)
  {
    unicode_field_instruction* new_inst = new (alloc_) unicode_field_instruction(*inst);
    new_inst->field_index(list_.size());
    list_.push_back(new_inst);
  }

  virtual void visit(const byte_vector_field_instruction* inst, void*)
  {
    byte_vector_field_instruction* new_inst = new (alloc_) byte_vector_field_instruction(*inst);
    new_inst->field_index(list_.size());
    list_.push_back(new_inst);
  }

  virtual void visit(const int32_vector_field_instruction* inst, void*)
  {
    int32_vector_field_instruction* new_inst = new (alloc_) int32_vector_field_instruction(*inst);
    new_inst->field_index(list_.size());
    list_.push_back(new_inst);
  }

  virtual void visit(const uint32_vector_field_instruction* inst, void*)
  {
    uint32_vector_field_instruction* new_inst = new (alloc_) uint32_vector_field_instruction(*inst);
    new_inst->field_index(list_.size());
    list_.push_back(new_inst);
  }

  virtual void visit(const int64_vector_field_instruction* inst, void*)
  {
    int64_vector_field_instruction* new_inst = new (alloc_) int64_vector_field_instruction(*inst);
    new_inst->field_index(list_.size());
    list_.push_back(new_inst);
  }

  virtual void visit(const uint64_vector_field_instruction* inst, void*)
  {
    uint64_vector_field_instruction* new_inst = new (alloc_) uint64_vector_field_instruction(*inst);
    new_inst->field_index(list_.size());
    list_.push_back(new_inst);
  }

  virtual void visit(const group_field_instruction* inst, void*)
  {
    group_field_instruction* new_inst = new (alloc_) group_field_instruction(*inst);
    new_inst->field_index(list_.size());
    list_.push_back(new_inst);
  }

  virtual void visit(const sequence_field_instruction* inst, void*)
  {
    sequence_field_instruction* new_inst = new (alloc_) sequence_field_instruction(*inst);
    new_inst->field_index(list_.size());
    list_.push_back(new_inst);
  }

  virtual void visit(const templateref_instruction* inst, void*)
  {
    templateref_instruction* new_inst = new (alloc_) templateref_instruction(*inst);
    new_inst->field_index(list_.size());
    list_.push_back(new_inst);
  }

  virtual void visit(const template_instruction* inst, void*)
  {
    for (std::size_t i = 0; i < inst->subinstructions_count(); ++i) {
      inst->subinstruction(i)->accept(*this, 0);
    }
  }

};

class templates_loader
  : public FastXMLVisitor
{
  std::deque<instruction_list_t> stack_;
  typedef const field_instruction* instruction_cptr;
  templates_description* definition_;
  arena_allocator* alloc_;
  const char* cpp_ns_;
  template_registry* registry_;
  bool is_fix_protocol_;

public:
  templates_loader(templates_description* definition,
                   const char*            cpp_ns,
                   template_registry*     registry)
    : definition_(definition)
    , alloc_(&registry->impl_->alloc_)
    , cpp_ns_(new_string(cpp_ns))
    , registry_(registry)
    , is_fix_protocol_(false)
  {
  }

  instruction_list_t& current()
  {
    return stack_.back();
  }

  uint16_t current_index() const
  {
    return stack_.back().size();
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

  bool ensure_instruction_id(uint32_t id, instruction_cptr* first, instruction_cptr* last)
  {
    instruction_cptr* itr = first;
    for (; itr != last; ++itr) {
      if ( (*itr)->id() == id )
        break;
    }

    if (itr != last) {
      // found the instruction with specified id
      if (itr != first) {
        std::swap(*first, *itr);
      }
      return true;
    }
    return false;
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
                            const char*&       initialValue)
  {
    fieldOp = operator_none;
    opContext = 0;
    initialValue = 0;

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
      std::string opContext_dict = get_optional_attr(*fieldOpElem, "dictionary", "");
      if (!opContext_key.empty() || !opContext_dict.empty()) {
        opContext = new (*alloc_)op_context_t;
        opContext->key_ = new_string(opContext_key.c_str());
        opContext->ns_ = new_string(get_optional_attr(*fieldOpElem, "ns", ""));
        opContext->dictionary_ = new_string(opContext_dict.c_str());
      }
      initialValue = get_optional_attr(*fieldOpElem, "value", 0);
      return true;
    }
    return false;
  }

public:
  virtual bool VisitEnterTemplates(const XMLElement & element)
  {
    const char* xmlns = get_optional_attr(element, "xmlns", "");

    const char fix_protocol_xmlns[] = "http://www.fixprotocol.org/";
    if (std::strncmp(xmlns, fix_protocol_xmlns, sizeof(fix_protocol_xmlns)-1) == 0)
      is_fix_protocol_ = true;

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
      get_typeRef_ns(element),
      cpp_ns_
      );
    stack_.pop_back();
    current().push_back(instruction);
    registry_->impl_->add(current_context().ns_.c_str(), instruction);
    return true;
  }

  virtual bool VisitTemplateRef(const XMLElement & element,
                                const std::string& name_attr,
                                std::size_t /*index*/)
  {
    templateref_instruction* instruction;
    const XMLElement* parent = element.Parent()->ToElement();

    if (name_attr.size()) {
      std::string ns = get_optional_attr(element, "ns", current_context().ns_.c_str());
      template_instruction* target =
        registry_->impl_->find( ns.c_str(), name_attr.c_str());

      if (target == 0) {
        BOOST_THROW_EXCEPTION(template_not_found_error(name_attr.c_str(),
                                                       get_optional_attr(*parent, "name", "")));
      }

      if (current_index() == 0) {
        // if the templateRef is the first in a group or sequence, we don't need the clone the
        // individual field because the field_index remains the same
        for (size_t i = 0; i < target->subinstructions_count(); ++i) {
          const field_instruction* sub_inst = target->subinstruction(i);
          current().push_back(const_cast<field_instruction*>(sub_inst));
        }
      }
      else if (target->subinstructions_count() > 0) {
        // In this case, we do need the clone the subfield instructions because the field
        // index would be different from those in the referenced template.
        instruction_cloner cloner(current(), *alloc_);
        target->accept(cloner, 0);
      }
    }
    else {
      presence_enum_t optional = presence_mandatory;

      if (element.NextSibling() == 0 && element.PreviousSibling() == 0) {

        if (strcmp(parent->Name(), "group") == 0 &&
            strcmp("optional", get_optional_attr(*parent, "presence", "mandatory")) ==0)
        {
          optional = presence_optional;
        }
      }

      instruction = new (*alloc_)templateref_instruction(
        current_index(),
        optional);

      current().push_back(instruction);

    }

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
                              std::size_t /*index*/)
  {
    instruction_cptr* subinstructions =  current_instructions();
    std::size_t subinstructions_count = current().size();
    stack_.pop_back();

    group_field_instruction* instruction = new (*alloc_)group_field_instruction (
      current_index(),
      get_presence(element),
      get_id(element),
      new_string(name_attr.c_str()),
      get_ns(element),
      get_dictionary(element),
      subinstructions,
      subinstructions_count,
      get_typeRef_name(element),
      get_typeRef_ns(element)
      );

    const XMLElement* child = only_child_templateRef(element);
    if (child) {
      const char* target_name = child->Attribute("name", 0);
      if (target_name) {
        const char* target_ns = get_optional_attr(*child, "ns",  current_context().ns_.c_str());

        template_instruction* target =
          registry_->impl_->find(target_ns, target_name);

        if (target == 0)
          BOOST_THROW_EXCEPTION(template_not_found_error(target_name, name_attr.c_str()));

        instruction->ref_template(target);
      }
    }

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
                                 std::size_t /*index*/)
  {
    const XMLElement* length_element = element.FirstChildElement("length");
    uint32_field_instruction* length_instruction = 0;

    operator_enum_t fieldOp= operator_none;
    uint32_t id = 0;
    op_context_t* opContext=0;
    const char* initial_value_str;
    std::string length_name = name_attr + "___length___";
    std::string ns;
    int_value_storage<uint32_t> initial_value;

    if (length_element) {
      id = get_id(*length_element);
      get_field_attributes(*length_element, name_attr, fieldOp, opContext, initial_value_str );

      length_name = get_optional_attr(*length_element, "name", "");
      ns = get_ns(*length_element);
      if (initial_value_str) {
        initial_value = int_value_storage<uint32_t>(boost::lexical_cast<uint32_t>(initial_value_str));
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

    instruction_cptr* subinstructions =  current_instructions();
    std::size_t subinstructions_count = current().size();
    stack_.pop_back();

    sequence_field_instruction* instruction = new (*alloc_)sequence_field_instruction(
      current_index(),
      get_presence(element),
      get_id(element),
      new_string(name_attr.c_str()),
      get_ns(element),
      get_dictionary(element),
      subinstructions,
      subinstructions_count,
      length_instruction,
      get_typeRef_name(element),
      get_typeRef_ns(element));

    const XMLElement* child = only_child_templateRef(element);
    if (child) {
      const char* target_name = child->Attribute("name", 0);
      if (target_name) {
        const char* target_ns = get_optional_attr(*child, "ns", current_context().ns_.c_str());
        template_instruction* target =
          registry_->impl_->find(target_ns, target_name);

        if (target == 0)
          BOOST_THROW_EXCEPTION(template_not_found_error(target_name, name_attr.c_str()));

        instruction->ref_template(target);
      }
    }

    current().push_back(instruction);
    return true;
  }

  template <typename INT_TYPE>
  void gen_integer_instruction(const XMLElement & element,
                               const std::string& name_attr,
                               std::size_t /*index*/)
  {
    operator_enum_t fieldOp;
    op_context_t* opContext;
    const char* initial_value_str;

    int_value_storage<INT_TYPE> initial_value;

    get_field_attributes(element, name_attr, fieldOp, opContext, initial_value_str );

    if (initial_value_str) {
      initial_value = int_value_storage<INT_TYPE>(boost::lexical_cast<INT_TYPE>(initial_value_str));
    }

    typedef typename instruction_trait<INT_TYPE>::type intruction_t;
    intruction_t* instruction= new (*alloc_)intruction_t  (
      current_index(),
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
                            std::size_t /*index*/)
  {
    bool is_unsigned = element.Name()[0] == 'u';
    if (is_unsigned) {
      if (integer_bits == 64)
        gen_integer_instruction<uint64_t>(element, name_attr, current_index());
      else
        gen_integer_instruction<uint32_t>(element, name_attr, current_index());
    }
    else {
      if (integer_bits == 64)
        gen_integer_instruction<int64_t>(element, name_attr, current_index());
      else
        gen_integer_instruction<int32_t>(element, name_attr, current_index());
    }
    return true;
  }

  virtual bool VisitDecimal(const XMLElement & element,
                            const std::string& name_attr,
                            std::size_t /*index*/)
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
        const char* mantissa_initial_value_str;

        get_field_attributes(*mantissa_element,
                             name_attr + "_mantissa",
                             mantissa_fieldOp,
                             mantissa_opContext,
                             mantissa_initial_value_str);


        int_value_storage<int64_t> mantissa_initial_value;

        if (mantissa_initial_value_str) {
          mantissa_initial_value = int_value_storage<int64_t>(boost::lexical_cast<int64_t>(mantissa_initial_value_str));
        }

        mantissa_instruction = new (*alloc_)mantissa_field_instruction(
          mantissa_fieldOp,
          mantissa_opContext,
          mantissa_initial_value
          );
      }

      operator_enum_t exponent_fieldOp = operator_none;
      op_context_t* exponent_opContext = 0;
      decimal_value_storage exponent_initial_value;

      if (exponent_element) {

        const char* exponent_initial_value_str;
        get_field_attributes(*exponent_element, name_attr + "_exponent",
                             exponent_fieldOp,
                             exponent_opContext,
                             exponent_initial_value_str);

        if (exponent_initial_value_str) {
          short exp  = 128;
          try {
            exp = boost::lexical_cast<short>(exponent_initial_value_str);
          }
          catch (...) {
          }

          if (exp > 63 || exp < -63) {
            BOOST_THROW_EXCEPTION(fast_dynamic_error("D11") << reason_info(std::string("Invalid exponent initial value: ") +  exponent_initial_value_str ) );
          }
          exponent_initial_value = decimal_value_storage(0, static_cast<uint8_t>(exp));
        }
      }

      instruction = new (*alloc_)decimal_field_instruction(
        current_index(),
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
      const char* initial_value_str;
      decimal_value_storage initial_value;

      get_field_attributes(element, name_attr, fieldOp, opContext, initial_value_str);

      if (initial_value_str) {
        initial_value = decimal_value_storage(boost::lexical_cast<decimal_value_storage>(initial_value_str));
      }

      instruction = new (*alloc_)decimal_field_instruction(
        current_index(),
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

  virtual bool VisitString(const XMLElement & element,
                           const std::string& name_attr,
                           std::size_t /*index*/)
  {
    const char* charset =  get_optional_attr(element, "charset", "ascii");
    operator_enum_t fieldOp;
    op_context_t* opContext;
    const char* initial_value_str;

    get_field_attributes(element, name_attr, fieldOp, opContext, initial_value_str );

    string_value_storage initial_value;
    if (initial_value_str)
      initial_value = string_value_storage(new_string(initial_value_str));

    field_instruction* instruction = 0;

    if (strcmp(charset, "ascii") == 0) {
      instruction= new (*alloc_)ascii_field_instruction  (
        current_index(),
        fieldOp,
        get_presence(element),
        get_id(element),
        new_string(name_attr.c_str()),
        get_ns(element),
        opContext,
        initial_value
        );

    }
    else {

      uint32_t length_id =0;
      const char* length_name=0;
      const char* length_ns=0;
      const XMLElement* length_element = element.FirstChildElement("length");
      if (length_element) {
        length_id = get_id(*length_element);
        length_name = new_string(get_optional_attr(*length_element, "name", ""));
        length_ns = get_ns(*length_element);
      }


      instruction= new (*alloc_)unicode_field_instruction  (
        current_index(),
        fieldOp,
        get_presence(element),
        get_id(element),
        new_string(name_attr.c_str()),
        get_ns(element),
        opContext,
        initial_value,
        length_id,
        length_name,
        length_ns
        );
    }

    current().push_back(instruction);
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
                               std::size_t /*index*/)
  {
    operator_enum_t fieldOp;
    op_context_t* opContext;
    const char* initial_value_str;

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

    unsigned char* initial_value_buffer=0;
    int32_t initial_value_len=0;

    if (initial_value_str) {
      initial_value_buffer = static_cast<unsigned char*>(alloc_->allocate(strlen(initial_value_str)/2+1));
      initial_value_len = hex2binary(initial_value_str, initial_value_buffer);
      if (initial_value_len == -1) {
        BOOST_THROW_EXCEPTION(fast_dynamic_error("D11") << reason_info(std::string("Invalid byteVector initial value: ") +  initial_value_str ) );
      }
    }

    byte_vector_value_storage initial_value;
    if (initial_value_str)
      initial_value = byte_vector_value_storage(initial_value_buffer, initial_value_len);
    byte_vector_field_instruction* instruction = new (*alloc_)byte_vector_field_instruction(
      current_index(),
      fieldOp,
      get_presence(element),
      get_id(element),
      new_string(name_attr.c_str()),
      get_ns(element),
      opContext,
      initial_value,
      length_id,
      length_name,
      length_ns
      );

    current().push_back(instruction);
    return true;
  }

  template <typename INT_TYPE>
  void gen_int_vector_instruction(const XMLElement & element,
                                  const std::string& name_attr,
                                  std::size_t)
  {

    typedef vector_field_instruction<INT_TYPE> intruction_t;
    intruction_t* instruction= new (*alloc_)intruction_t  (
      current_index(),
      get_presence(element),
      get_id(element),
      new_string(name_attr.c_str()),
      get_ns(element));

    current().push_back(instruction);
  }

  virtual bool VisitIntVector(const XMLElement & element, int integer_bits, const std::string& name_attr, std::size_t)
  {

    bool is_unsigned = element.Name()[0] == 'u';
    if (is_unsigned) {
      if (integer_bits == 64)
        gen_int_vector_instruction<uint64_t>(element, name_attr, current_index());
      else
        gen_int_vector_instruction<uint32_t>(element, name_attr, current_index());
    }
    else {
      if (integer_bits == 64)
        gen_int_vector_instruction<int64_t>(element, name_attr, current_index());
      else
        gen_int_vector_instruction<int32_t>(element, name_attr, current_index());
    }
    return true;
  }

};


dynamic_templates_description::dynamic_templates_description(const char*        xml_content,
                                                             const char*        cpp_ns,
                                                             template_registry* registry)
{
  tinyxml2::XMLDocument document;
  if (document.Parse(xml_content) == 0) {
    templates_loader loader(this, cpp_ns, registry);
    document.Accept(&loader);
  }
  else {
    BOOST_THROW_EXCEPTION(fast_static_error("S1"));
  }
}

}
