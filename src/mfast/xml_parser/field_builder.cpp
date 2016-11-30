// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.
#include "field_builder.h"
#include "field_op.h"
#include "../exceptions.h"
#include <boost/tokenizer.hpp>
#include "mfast/field_instructions.h"

using namespace tinyxml2;

namespace mfast {
namespace xml_parser {
struct tag_referenced_by;
struct tag_template_name;

typedef boost::error_info<tag_referenced_by, std::string> referenced_by_info;
typedef boost::error_info<tag_template_name, std::string> template_name_info;

class template_not_found_error : public fast_dynamic_error {
public:
  template_not_found_error(const char *template_name, const char *referenced_by)
      : fast_dynamic_error("D8") {
    *this << template_name_info(template_name)
          << referenced_by_info(referenced_by);
  }
};

#ifndef NDEBUG

void print_node(const XMLElement *element, int indent)
{
  for (int i = 0; i < indent; ++i)
    std::cout << " ";
  std::cout << "node " << element->Name() ;
  const XMLAttribute *attr = element->FirstAttribute();
  while (attr) {
    std::cout << " " << attr->Name() << "=" << attr->Value();
    attr = attr->Next();
  }
  std::cout << "\n";

  const XMLElement *elem = element->FirstChildElement();
  while (elem){
    print_node(elem, indent+4);
    elem = elem->NextSiblingElement();
  }
}

#endif

const char *field_builder::name() const { return name_; }

std::size_t field_builder::num_instructions() const {
  return instructions_.size();
}

void field_builder::add_instruction(const field_instruction *inst) {
  instructions_.push_back(inst);
}

const char *field_builder::resolve_field_type(const XMLElement &element) {
  field_type_name_ = element.Name();
  content_element_ = &element;

  if (std::strcmp(field_type_name_, "field") == 0) {
    content_element_ = element.FirstChildElement();
    if (content_element_) {
      field_type_name_ = content_element_->Name();
      if (strcmp(field_type_name_, "type") == 0) {
        field_type_name_ = name_;
        name_ = nullptr;
        fast_xml_attributes::set(content_element_->FirstAttribute());

        if (name_ == nullptr)
          throw std::runtime_error("type element does not have a name");
        std::swap(field_type_name_, name_);
      } else {
        fast_xml_attributes::set(content_element_->FirstAttribute());
      }
    } else {
      throw std::runtime_error("field element must have a  child element");
    }
  }

  resolved_ns_ = ns_;
  if (resolved_ns_ == nullptr && parent_) {
    resolved_ns_ = parent_->resolved_ns();
  }

  return field_type_name_;
}

const field_instruction *field_builder::find_prototype(const char *type_name) {
  if (type_name == nullptr) {
    BOOST_THROW_EXCEPTION(fast_static_error("S1")
                          << reason_info("no field type specified"));
  }
  const field_instruction *instruction = nullptr;

  if (std::strcmp(type_name, "string") == 0) {
    if (this->charset_ == nullptr)
      this->charset_ = get_optional_attr(element_, "charset", "ascii");

    if (std::strcmp(this->charset_, "unicode") == 0) {
      static unicode_field_instruction prototype(
          operator_none, presence_mandatory, 0, nullptr, "", nullptr,
          string_value_storage(), 0, "", "");
      instruction = &prototype;
    }
  } else if (std::strcmp(type_name, "templateRef") == 0) {
    return templateref_instruction::default_instruction()[0];
  } else if (std::strcmp(type_name, "typeRef") == 0 ||
             std::strcmp(type_name, "length") == 0) {
    return nullptr;
  }

  if (instruction == nullptr) {
    instruction = this->find_type(ns_, type_name);
  }

  if (instruction == nullptr) {
    BOOST_THROW_EXCEPTION(
        fast_static_error("S1") << reason_info(
            (std::string("Invalid field type specified : ") + type_name)));
  }

  return instruction;
}

field_builder::field_builder(field_builder_base *parent,
                             const XMLElement &element)
    : fast_xml_attributes(element.FirstAttribute()),
      field_builder_base(parent->registry(), parent->local_types()),
      element_(element), parent_(parent) {}

field_builder::field_builder(field_builder_base *parent,
                             const XMLElement &element, const char *name)
    : fast_xml_attributes(element.FirstAttribute()),
      field_builder_base(parent->registry(), parent->local_types()),
      element_(element), parent_(parent) {
  name_ = name;
}

void field_builder::build() {
  const field_instruction *prototype =
      find_prototype(resolve_field_type(element_));
  if (prototype) {
    prototype->accept(*this, nullptr);
  }
}

template <typename IntType>
void field_builder::build_integer(const int_field_instruction<IntType> *inst) {
  field_op fop(inst, &element_, alloc());

  field_instruction *instruction = new (alloc()) int_field_instruction<IntType>(
      fop.op_, get_presence(inst), get_id(inst), get_name(alloc()),
      get_ns(inst, alloc()), fop.context_,
      int_value_storage<IntType>(fop.initial_value_), parse_tag(inst));

  parent_->add_instruction(instruction);
}

void field_builder::visit(const int32_field_instruction *inst, void *) {
  build_integer(inst);
}

void field_builder::visit(const uint32_field_instruction *inst, void *) {
  build_integer(inst);
}

void field_builder::visit(const int64_field_instruction *inst, void *) {
  build_integer(inst);
}

void field_builder::visit(const uint64_field_instruction *inst, void *) {
  build_integer(inst);
}

void field_builder::visit(const decimal_field_instruction *inst, void *) {
  const mantissa_field_instruction *base_mantissa_instruction =
      inst->mantissa_instruction();
  const XMLElement *mantissa_element = element_.FirstChildElement("mantissa");
  const XMLElement *exponent_element = element_.FirstChildElement("exponent");
  field_instruction *instruction;
  if (base_mantissa_instruction || mantissa_element || exponent_element) {

    mantissa_field_instruction *mantissa_instruction = nullptr;

    if (base_mantissa_instruction == nullptr) {
      static const mantissa_field_instruction mantissa_prototype(
          operator_none, nullptr, int_value_storage<int64_t>(0));
      base_mantissa_instruction = &mantissa_prototype;
    }

    if (mantissa_element) {
      field_op mantissa_op(base_mantissa_instruction, mantissa_element,
                           alloc());

      mantissa_instruction = new (alloc()) mantissa_field_instruction(
          mantissa_op.op_, mantissa_op.context_,
          int_value_storage<int64_t>(mantissa_op.initial_value_));
    } else {
      mantissa_instruction =
          new (alloc()) mantissa_field_instruction(*base_mantissa_instruction);
    }

    field_op exponent_op(inst, exponent_element, alloc());

    instruction = new (alloc()) decimal_field_instruction(
        exponent_op.op_, get_presence(inst), get_id(inst), get_name(alloc()),
        get_ns(inst, alloc()), exponent_op.context_, mantissa_instruction,
        decimal_value_storage(exponent_op.initial_value_), parse_tag(inst));
  } else {

    field_op decimal_op(inst, &element_, alloc());

    instruction = new (alloc()) decimal_field_instruction(
        decimal_op.op_, get_presence(inst), get_id(inst), get_name(alloc()),
        get_ns(inst, alloc()), decimal_op.context_,
        decimal_value_storage(decimal_op.initial_value_), parse_tag(inst));
  }

  parent_->add_instruction(instruction);
}

void field_builder::visit(const ascii_field_instruction *inst, void *) {
  field_op fop(inst, &element_, alloc());
  field_instruction *instruction = new (alloc()) ascii_field_instruction(
      fop.op_, get_presence(inst), get_id(inst), get_name(alloc()),
      get_ns(inst, alloc()), fop.context_,
      string_value_storage(fop.initial_value_), parse_tag(inst));
  parent_->add_instruction(instruction);
}

void field_builder::visit(const unicode_field_instruction *inst, void *) {
  field_op fop(inst, &element_, alloc());

  const XMLAttribute *length_attributes = nullptr;
  const XMLElement *length_element = element_.FirstChildElement("length");

  if (length_element) {
    length_attributes = length_element->FirstAttribute();
  }

  fast_xml_attributes length_attrs(length_attributes);

  field_instruction *instruction = new (alloc()) unicode_field_instruction(
      fop.op_, get_presence(inst), get_id(inst), get_name(alloc()),
      get_ns(inst, alloc()), fop.context_,
      string_value_storage(fop.initial_value_),
      get_length_id(inst, length_attrs), get_length_name(inst, length_attrs),
      get_length_ns(inst, length_attrs), parse_tag(inst));
  parent_->add_instruction(instruction);
}

void field_builder::visit(const byte_vector_field_instruction *inst, void *) {
  field_op fop(inst, &element_, alloc());

  const XMLAttribute *length_attributes = nullptr;
  const XMLElement *length_element = element_.FirstChildElement("length");

  if (length_element) {
    length_attributes = length_element->FirstAttribute();
  }

  fast_xml_attributes length_attrs(length_attributes);

  field_instruction *instruction = new (alloc()) byte_vector_field_instruction(
      fop.op_, get_presence(inst), get_id(inst), get_name(alloc()),
      get_ns(inst, alloc()), fop.context_,
      string_value_storage(fop.initial_value_),
      get_length_id(inst, length_attrs), get_length_name(inst, length_attrs),
      get_length_ns(inst, length_attrs), parse_tag(inst));
  parent_->add_instruction(instruction);
}

void field_builder::visit(const int32_vector_field_instruction *inst, void *) {
  field_instruction *instruction = new (alloc()) int32_vector_field_instruction(
      get_presence(inst), get_id(inst), get_name(alloc()),
      get_ns(inst, alloc()), parse_tag(inst));
  parent_->add_instruction(instruction);
}

void field_builder::visit(const uint32_vector_field_instruction *inst, void *) {
  field_instruction *instruction = new (alloc())
      uint32_vector_field_instruction(get_presence(inst), get_id(inst),
                                      get_name(alloc()), get_ns(inst, alloc()),
                                      parse_tag(inst));
  parent_->add_instruction(instruction);
}

void field_builder::visit(const int64_vector_field_instruction *inst, void *) {
  field_instruction *instruction = new (alloc()) int64_vector_field_instruction(
      get_presence(inst), get_id(inst), get_name(alloc()),
      get_ns(inst, alloc()), parse_tag(inst));
  parent_->add_instruction(instruction);
}

void field_builder::visit(const uint64_vector_field_instruction *inst, void *) {
  field_instruction *instruction = new (alloc())
      uint64_vector_field_instruction(get_presence(inst), get_id(inst),
                                      get_name(alloc()), get_ns(inst, alloc()),
                                      parse_tag(inst));
  parent_->add_instruction(instruction);
}

void field_builder::visit(const templateref_instruction *, void *) {
  templateref_instruction *instruction;

  if (name_) {
    const char *resolved_ns =
        get_optional_attr(element_, "templateNs", parent_->resolved_ns());
    const template_instruction *target =
        dynamic_cast<const template_instruction *>(
            this->find_type(resolved_ns, name_));

    if (target == nullptr) {
      BOOST_THROW_EXCEPTION(template_not_found_error(name_, parent_->name()));
    }

    // if (parent_->num_instructions() == 0) {
    //   // if the templateRef is the first in a group or sequence, we don't
    //   need the clone the
    //   // individual field because the field_index remains the same
    //   for (size_t i = 0; i < target->subinstructions().size(); ++i) {
    //     const field_instruction* sub_inst = target->subinstruction(i);
    //     parent_->add_instruction(sub_inst);
    //   }
    // }
    // else if (target->subinstructions().size() > 0) {
    //   // In this case, we do need the clone the subfield instructions because
    //   the field
    //   // index would be different from those in the referenced template.
    //   field_instruction* new_inst;
    //   for (size_t i = 0; i < target->subinstructions().size(); ++i) {
    //     new_inst = target->subinstruction(i)->clone(alloc());
    //     parent_->add_instruction(new_inst);
    //   }
    //   if (target->subinstructions().size() == 1 && new_inst->field_type() ==
    //   field_type_sequence)
    //   {
    //     static_cast<sequence_field_instruction*>(new_inst)->ref_instruction(target);
    //   }
    // }

    if (target->subinstructions().size() == 1 &&
        target->subinstruction(0)->field_type() == field_type_sequence) {
      field_instruction *new_inst = target->subinstruction(0)->clone(alloc());
      parent_->add_instruction(new_inst);
      static_cast<sequence_field_instruction *>(new_inst)
          ->ref_instruction(target);
    } else {
      for (size_t i = 0; i < target->subinstructions().size(); ++i) {
        const field_instruction *sub_inst = target->subinstruction(i);
        parent_->add_instruction(sub_inst);
      }
    }
  } else {

    instruction = new (alloc()) templateref_instruction(
        static_cast<uint16_t>(parent_->num_instructions()));

    parent_->add_instruction(instruction);
  }
}

instructions_view_t field_builder::build_subfields() {
  const XMLElement *child = content_element_->FirstChildElement();
  while (child != nullptr) {

    field_builder builder(this, *child);
    builder.build();
    child = child->NextSiblingElement();
  }

  auto result = new (alloc()) const_instruction_ptr_t[this->num_instructions()];
  std::copy(instructions_.begin(), instructions_.end(), result);
  return instructions_view_t(result, this->num_instructions());
}

const group_field_instruction *field_builder::get_sole_templateref() {
  const XMLElement *child = content_element_->FirstChildElement();
  if (strcmp(child->Name(), "templateRef") == 0 &&
      child->NextSibling() == nullptr) {
    const char *target_name = child->Attribute("name", nullptr);
    if (target_name) {
      const char *target_ns =
          get_optional_attr(*child, "templateNs", parent_->resolved_ns());

      const group_field_instruction *target =
          dynamic_cast<const group_field_instruction *>(
              this->find_type(target_ns, target_name));

      if (target == nullptr)
        BOOST_THROW_EXCEPTION(
            template_not_found_error(target_name, this->name()));

      return target;
    }
  }
  return nullptr;
}

void field_builder::visit(const group_field_instruction *inst, void *) {
  instructions_view_t subinstructions = inst->subinstructions();

  if (inst->subinstructions().size() == 0) {
    subinstructions = build_subfields();
  }

  auto instruction = new (alloc()) group_field_instruction(
      get_presence(inst), get_id(inst), get_name(alloc()),
      get_ns(inst, alloc()), get_dictionary(inst), subinstructions,
      get_typeRef_name(element_), get_typeRef_ns(element_), inst->cpp_ns(),
      parse_tag(inst));

  if (inst->subinstructions().size() == 0) {
    instruction->ref_instruction(get_sole_templateref());
  } else {
    instruction->ref_instruction(inst);
  }

  parent_->add_instruction(instruction);
}

const uint32_field_instruction *
field_builder::get_length_instruction(const sequence_field_instruction *inst) {
  const uint32_field_instruction *length_instruction =
      inst->length_instruction();

  const XMLElement *length_element =
      content_element_->FirstChildElement("length");

  if (length_element) {

    fast_xml_attributes length_attributes(length_element->FirstAttribute());

    field_op length_fop(length_instruction, length_element, alloc());

    length_instruction = new (alloc()) uint32_field_instruction(
        length_fop.op_, get_presence(inst),
        length_attributes.get_id(length_instruction),
        length_attributes.get_name(alloc()),
        length_attributes.get_ns(length_instruction, alloc()),
        length_fop.context_,
        int_value_storage<uint32_t>(length_fop.initial_value_));
  } else if (length_instruction->optional() !=
             (get_presence(inst) == mfast::presence_optional)) {
    length_instruction = new (alloc()) uint32_field_instruction(
        length_instruction->field_operator(), get_presence(inst),
        length_instruction->id(), length_instruction->name(),
        length_instruction->ns(), length_instruction->op_context(),
        int_value_storage<uint32_t>(length_instruction->initial_value()));
  }
  return length_instruction;
}

void field_builder::visit(const sequence_field_instruction *inst, void *) {
  instructions_view_t subinstructions = inst->subinstructions();

  sequence_field_instruction *instruction;
  const group_field_instruction *element_instruction = nullptr;

  if (inst->subinstructions().size() == 0) {
    subinstructions = build_subfields();
    element_instruction = get_sole_templateref();
    if (element_instruction == nullptr && subinstructions.size() == 1 &&
        subinstructions[0]->name()[0] == 0) {
      element_instruction =
          dynamic_cast<const group_field_instruction *>(subinstructions[0]);
      if (element_instruction)
        element_instruction = static_cast<const group_field_instruction *>(
            element_instruction->ref_instruction());
    }
  }

  const group_field_instruction *ref_inst =
      inst->subinstructions().size() == 0 ? nullptr : inst;

  instruction = new (alloc()) sequence_field_instruction(
      get_presence(inst), get_id(inst), get_name(alloc()),
      get_ns(inst, alloc()), get_dictionary(inst), subinstructions,
      element_instruction, ref_inst, get_length_instruction(inst),
      get_typeRef_name(element_), get_typeRef_ns(element_), inst->cpp_ns(),
      parse_tag(inst));

  parent_->add_instruction(instruction);
}

void field_builder::visit(const template_instruction *inst, void *) {
  bool reset = false;
  const XMLAttribute *reset_attr = element_.FindAttribute("scp:reset");
  if (reset_attr == nullptr)
    reset_attr = element_.FindAttribute("reset");

  if (reset_attr) {
    if (strcmp(reset_attr->Value(), "true") == 0 ||
        strcmp(reset_attr->Value(), "yes") == 0)
      reset = true;
  }

  auto instruction = new (alloc()) template_instruction(
      id_ ? boost::lexical_cast<uint32_t>(id_) : 0, string_dup(name_, alloc()),
      string_dup(ns_, alloc()), string_dup(templateNs_, alloc()),
      string_dup(dictionary_, alloc()), build_subfields(), reset,
      get_typeRef_name(element_), get_typeRef_ns(element_), inst->cpp_ns(),
      parse_tag(inst));

  parent_->add_template(this->resolved_ns(), instruction);
}

void field_builder::add_template(const char *, template_instruction *inst) {
  BOOST_THROW_EXCEPTION(fast_static_error("S1")
                        << reason_info("template cannot be nested")
                        << template_name_info(inst->name())
                        << referenced_by_info(parent_->name()));
}

bool parse_enum_value(const char **enum_element_names,
                      const uint64_t *enum_element_values,
                      uint64_t num_elements, const char *value_name,
                      uint64_t &result) {

  for (uint64_t i = 0; i < num_elements; ++i) {
    if (std::strcmp(enum_element_names[i], value_name) == 0) {
      if (enum_element_values)
        result = enum_element_values[i];
      else
        result = i;
      return true;
    }
  }

  return false;
}

bool parse_enum_value(const enum_field_instruction *inst,
                      const char *value_name, uint64_t &result) {
  return parse_enum_value(inst->elements(), inst->element_values(),
                          inst->num_elements(), value_name, result);
}

struct tag_value;
typedef boost::error_info<tag_value, std::string> value_info;

void field_builder::visit(const enum_field_instruction *inst, void *) {

  const XMLElement *element = &this->element_;
  if (!field_op::find_field_op_element(*element))
    element = content_element_;
  field_op fop(inst, element, alloc());

  const char **enum_element_names = inst->elements();
  uint64_t num_elements = inst->num_elements();
  const uint64_t *enum_element_values = inst->element_values();

  const char *init_value_str = nullptr;
  if (!fop.initial_value_.is_defined()) {
    // if the  defined flag is false, the content value is parsed string from
    // XML
    init_value_str = fop.initial_value_.get<const char *>();
  }

  if (enum_element_names == nullptr) {

    std::deque<const char *> names;
    std::deque<uint64_t> values;

    const XMLElement *xml_element =
        content_element_->FirstChildElement("element");
    for (; xml_element != nullptr;
         xml_element = xml_element->NextSiblingElement("element")) {
      const char *name_attr = xml_element->Attribute("name");
      if (name_attr != nullptr) {
        if (init_value_str && std::strcmp(name_attr, init_value_str) == 0) {
          fop.initial_value_.set<uint64_t>(names.size());
        }
        names.push_back(string_dup(name_attr, alloc()));

        const char *value_str = xml_element->Attribute("value");
        if (value_str) {
          uint64_t v = boost::lexical_cast<uint64_t>(value_str);
          if (values.empty() || v > values.back()) {
            values.push_back(v);
          }
        }
      } else {
        throw std::runtime_error("XML element must have a name attribute");
      }
    }

    if (values.size() && values.size() != names.size()) {
      throw std::runtime_error("Invalid value specification for enum elements");
    }

    num_elements = names.size();
    enum_element_names = static_cast<const char **>(
        alloc().allocate(names.size() * sizeof(const char *)));
    std::copy(names.begin(), names.end(), enum_element_names);

    if (values.size()) {
      uint64_t *values_array = static_cast<uint64_t *>(
          alloc().allocate(values.size() * sizeof(uint64_t)));
      std::copy(values.begin(), values.end(), values_array);
      enum_element_values = values_array;
    }
  } else if (init_value_str) {
    // In this case, the element names are already defined, but we haven't
    // decide what the specified
    // initial value is.

    uint64_t init_value;
    if (parse_enum_value(enum_element_names, enum_element_values, num_elements,
                         init_value_str, init_value)) {
      fop.initial_value_ =
          value_storage(0); // reset the storage to defined value
      fop.initial_value_.set<uint64_t>(init_value);
    } else {
      BOOST_THROW_EXCEPTION(
          fast_static_error("Unrecognized enum initial value : ")
          << value_info(init_value_str));
    }
  }

  if (!fop.initial_value_.is_defined()) {
    if (fop.initial_value_.get<const char *>() != nullptr) {
      std::string msg = "Invalid initial value for enum : ";
      throw std::runtime_error(msg + init_value_str);
    } else {
      // at this point if initial_value_ is still undefined, we should reset it
      // to zero
      fop.initial_value_.set<uint64_t>(0);
    }
  }

  auto instruction = new (alloc()) enum_field_instruction(
      fop.op_, get_presence(inst), get_id(inst), get_name(alloc()),
      get_ns(inst, alloc()), fop.context_,
      int_value_storage<uint64_t>(fop.initial_value_), enum_element_names,
      enum_element_values, num_elements,
      inst->elements_ == nullptr ? nullptr : inst, inst->cpp_ns(),
      parse_tag(inst));

  parent_->add_instruction(instruction);
}

instruction_tag field_builder::parse_tag(const field_instruction *inst) {
  uint64_t value = inst->tag().to_uint64();
  if (tag_) {
    try {
      value |= boost::lexical_cast<uint64_t>(tag_);
    } catch (...) {
      const enum_field_instruction *inst =
          dynamic_cast<const enum_field_instruction *>(
              this->find_type(nullptr, "mfast:tag"));
      if (inst) {
        // treat the input tag as a "|" delimited tokens

        boost::char_separator<char> sep("| ");
        std::string tag(tag_);
        boost::tokenizer<boost::char_separator<char>> tokens(tag, sep);
        for (auto &&t : tokens) {
          uint64_t result;
          if (parse_enum_value(inst, t.c_str(), result))
            value |= result;
          else
            BOOST_THROW_EXCEPTION(fast_error("invalid tag") << value_info(t));
        }
      }
    }
  }
  return instruction_tag(value);
}

} /* coder */

} /* mfast */
