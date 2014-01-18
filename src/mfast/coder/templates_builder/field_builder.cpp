#include "field_builder.h"
#include "field_op.h"
#include "instruction_cloner.h"
#include "../common/exceptions.h"

using namespace tinyxml2;
using namespace boost::assign; // bring 'map_list_of()' into scope


namespace mfast
{
namespace coder
{


const char* field_builder::name() const
{
  return name_;
}

std::size_t field_builder::num_instructions() const
{
  return instructions_.size();
}

void field_builder::add_instruction(const field_instruction* inst)
{
  instructions_.push_back(inst);
}

const char* field_builder::resolve_field_type(const XMLElement& element)
{
  field_type_name_ = element.Name();
  content_element_ = &element;

  if (std::strcmp(field_type_name_, "field") == 0 ) {
    content_element_ = element.FirstChildElement("type");
    if (content_element_) {
      field_type_name_ = name_;
      name_ = 0;
      fast_xml_attributes::set(content_element_->FirstAttribute());

      if (name_ == 0)
        throw std::runtime_error("type element does not have a name");
      std::swap(field_type_name_, name_);
    }
    else {
      throw std::runtime_error("field element must have a type sub-element");
    }
  }

  is_optional_group_ =
    std::strcmp(field_type_name_, "group") == 0 && (presence_ != 0) && std::strcmp(presence_, "optional");

  resolved_ns_ = ns_;
  if (resolved_ns_ == 0 && parent_) {
    resolved_ns_ = parent_->resolved_ns();
  }

  return field_type_name_;
}

const field_instruction* field_builder::find_prototype(const char* type_name)
{
  if (type_name == 0)
  {
    BOOST_THROW_EXCEPTION(fast_static_error("S1") << reason_info("no field type specified"));
  }
  const field_instruction* instruction = 0;

  if (std::strcmp(type_name, "string") == 0) {
    if (this->charset_ == 0)
      this->charset_ = get_optional_attr(element_, "charset", "ascii");

    if (std::strcmp(this->charset_, "unicode") == 0 )
    {
      static unicode_field_instruction prototype (0,operator_none,presence_mandatory,0,0,"",0);
      instruction = &prototype;
    }
  }
  else if (std::strcmp(type_name, "templateRef") == 0)
  {
    return *templateref_instruction::default_instruction();
  }
  else if (std::strcmp(type_name, "typeRef") == 0 || std::strcmp(type_name, "length") == 0)
  {
    return 0;
  }

  if (instruction == 0) {
    instruction = this->find_type(ns_, type_name);
  }

  if ( instruction == 0 )
  {
    BOOST_THROW_EXCEPTION(fast_static_error("S1") << reason_info((std::string("Invalid field type specified : ") + type_name)));
  }

  return instruction;
}

field_builder::field_builder(field_builder_base* parent,
                             const XMLElement&   element)
  : fast_xml_attributes(element.FirstAttribute())
  , field_builder_base(parent->registry(),
                       parent->local_types())
  , element_(element)
  , parent_(parent)
{

  const field_instruction* prototype =
    find_prototype(resolve_field_type(element));

  if (prototype)
    prototype->accept(*this, 0);
}

field_builder::field_builder(field_builder_base* parent,
                             const XMLElement&   element,
                             const char*         name)
  : fast_xml_attributes(element.FirstAttribute())
  , field_builder_base(parent->registry(),
                       parent->local_types())
  , element_(element)
  , parent_(parent)
{
  name_ = name;
  const field_instruction* prototype =
    find_prototype(resolve_field_type(element));

  prototype->accept(*this, 0);
}

template <typename IntType>
void field_builder::build_integer(const int_field_instruction<IntType>* inst)
{
  field_op fop(inst, &element_, alloc());

  field_instruction* instruction = new (alloc())int_field_instruction<IntType> (
    parent_->num_instructions(),
    fop.op_,
    get_presence(inst),
    get_id(inst),
    get_name(alloc()),
    get_ns(inst, alloc()),
    fop.context_,
    int_value_storage<IntType>(fop.initial_value_)
    );

  parent_->add_instruction(instruction);
}

void field_builder::visit(const int32_field_instruction* inst, void*)
{
  build_integer(inst);
}

void field_builder::visit(const uint32_field_instruction* inst, void*)
{
  build_integer(inst);
}

void field_builder::visit(const int64_field_instruction* inst, void*)
{
  build_integer(inst);
}

void field_builder::visit(const uint64_field_instruction* inst, void*)
{
  build_integer(inst);
}

void field_builder::visit(const decimal_field_instruction* inst, void*)
{
  const XMLElement* mantissa_element = element_.FirstChildElement("mantissa");
  const XMLElement* exponent_element = element_.FirstChildElement("exponent");
  field_instruction* instruction;
  if (mantissa_element || exponent_element) {

    mantissa_field_instruction* mantissa_instruction =0;

    if (mantissa_element)
    {
      static const mantissa_field_instruction mantissa_instruction_prototype(operator_none, 0, int_value_storage<int64_t>(0));
      const mantissa_field_instruction* base_mantissa_instruction = inst->mantissa_instruction();
      if (mantissa_instruction == 0)
        base_mantissa_instruction = &mantissa_instruction_prototype;

      field_op mantissa_op(base_mantissa_instruction, mantissa_element, alloc());

      mantissa_instruction = new (alloc())mantissa_field_instruction(
        mantissa_op.op_,
        mantissa_op.context_,
        int_value_storage<int64_t>(mantissa_op.initial_value_));
    }

    field_op exponent_op(inst, exponent_element, alloc());

    instruction = new (alloc())decimal_field_instruction(
      parent_->num_instructions(),
      exponent_op.op_,
      get_presence(inst),
      get_id(inst),
      get_name(alloc()),
      get_ns(inst, alloc()),
      exponent_op.context_,
      mantissa_instruction,
      decimal_value_storage(exponent_op.initial_value_));
  }
  else {

    field_op decimal_op(inst, &element_, alloc());

    instruction = new (alloc())decimal_field_instruction(
      parent_->num_instructions(),
      decimal_op.op_,
      get_presence(inst),
      get_id(inst),
      get_name(alloc()),
      get_ns(inst, alloc()),
      decimal_op.context_,
      decimal_value_storage(decimal_op.initial_value_));
  }

  parent_->add_instruction(instruction);
}

void field_builder::visit(const ascii_field_instruction* inst, void*)
{
  field_op fop(inst, &element_, alloc());
  field_instruction* instruction = new (alloc())ascii_field_instruction  (
    parent_->num_instructions(),
    fop.op_,
    get_presence(inst),
    get_id(inst),
    get_name(alloc()),
    get_ns(inst, alloc()),
    fop.context_,
    string_value_storage(fop.initial_value_)
    );
  parent_->add_instruction(instruction);
}

void field_builder::visit(const unicode_field_instruction* inst, void*)
{
  field_op fop(inst, &element_, alloc());

  const XMLAttribute* length_attributes = 0;
  const XMLElement* length_element = element_.FirstChildElement("length");

  if (length_element) {
    length_attributes = length_element->FirstAttribute();
  }

  fast_xml_attributes length_attrs(length_attributes);

  field_instruction* instruction = new (alloc())unicode_field_instruction  (
    parent_->num_instructions(),
    fop.op_,
    get_presence(inst),
    get_id(inst),
    get_name(alloc()),
    get_ns(inst, alloc()),
    fop.context_,
    string_value_storage(fop.initial_value_),
    get_length_id(inst, length_attrs),
    get_length_name(inst, length_attrs),
    get_length_ns(inst, length_attrs)
    );
  parent_->add_instruction(instruction);
}

void field_builder::visit(const byte_vector_field_instruction* inst, void*)
{
  field_op fop(inst, &element_, alloc());

  const XMLAttribute* length_attributes = 0;
  const XMLElement* length_element = element_.FirstChildElement("length");

  if (length_element) {
    length_attributes = length_element->FirstAttribute();
  }

  fast_xml_attributes length_attrs(length_attributes);

  field_instruction* instruction = new (alloc())byte_vector_field_instruction  (
    parent_->num_instructions(),
    fop.op_,
    get_presence(inst),
    get_id(inst),
    get_name(alloc()),
    get_ns(inst, alloc()),
    fop.context_,
    string_value_storage(fop.initial_value_),
    get_length_id(inst, length_attrs),
    get_length_name(inst, length_attrs),
    get_length_ns(inst, length_attrs)
    );
  parent_->add_instruction(instruction);
}

void field_builder::visit(const int32_vector_field_instruction* inst, void*)
{
  field_instruction* instruction = new (alloc())int32_vector_field_instruction  (
    parent_->num_instructions(),
    get_presence(inst),
    get_id(inst),
    get_name(alloc()),
    get_ns(inst, alloc()));
  parent_->add_instruction(instruction);
}

void field_builder::visit(const uint32_vector_field_instruction* inst, void*)
{
  field_instruction* instruction = new (alloc())uint32_vector_field_instruction  (
    parent_->num_instructions(),
    get_presence(inst),
    get_id(inst),
    get_name(alloc()),
    get_ns(inst, alloc()));
  parent_->add_instruction(instruction);
}

void field_builder::visit(const int64_vector_field_instruction* inst, void*)
{
  field_instruction* instruction = new (alloc())int64_vector_field_instruction  (
    parent_->num_instructions(),
    get_presence(inst),
    get_id(inst),
    get_name(alloc()),
    get_ns(inst, alloc()));
  parent_->add_instruction(instruction);
}

void field_builder::visit(const uint64_vector_field_instruction* inst, void*)
{
  field_instruction* instruction = new (alloc())uint64_vector_field_instruction  (
    parent_->num_instructions(),
    get_presence(inst),
    get_id(inst),
    get_name(alloc()),
    get_ns(inst, alloc()));
  parent_->add_instruction(instruction);
}

void field_builder::visit(const templateref_instruction*, void*)
{
  templateref_instruction* instruction;

  if (name_) {
    const char* resolved_ns = get_optional_attr(element_, "ns", parent_->resolved_ns());
    const template_instruction* target =
      dynamic_cast<const template_instruction*> (this->find_type( resolved_ns, name_));

    if (target == 0) {
      BOOST_THROW_EXCEPTION(template_not_found_error(name_,
                                                     parent_->name()));
    }

    if (parent_->num_instructions() == 0) {
      // if the templateRef is the first in a group or sequence, we don't need the clone the
      // individual field because the field_index remains the same
      for (size_t i = 0; i < target->subinstructions_count(); ++i) {
        const field_instruction* sub_inst = target->subinstruction(i);
        parent_->add_instruction(sub_inst);
      }
    }
    else if (target->subinstructions_count() > 0) {
      // In this case, we do need the clone the subfield instructions because the field
      // index would be different from those in the referenced template.
      instruction_cloner cloner(*parent_, alloc());
      target->accept(cloner, 0);

    }
  }
  else {

    instruction = new (alloc())templateref_instruction(
      parent_->num_instructions());

    parent_->add_instruction(instruction);

  }
}

const_instruction_ptr_t*
field_builder::build_subfields()
{
  const XMLElement* child = content_element_->FirstChildElement();
  while (child != 0) {

    field_builder field(this, *child);
    child = child->NextSiblingElement();
  }

  const_instruction_ptr_t* result = new (alloc())const_instruction_ptr_t[this->num_instructions()];
  std::copy(instructions_.begin(), instructions_.end(), result);
  return result;
}

void field_builder::set_ref_instruction(group_field_instruction* instruction)
{
  const XMLElement* child = content_element_->FirstChildElement();
  if (strcmp(child->Name(), "templateRef") == 0 && child->NextSibling() == 0) {
    const char* target_name = child->Attribute("name", 0);
    if (target_name) {
      const char* target_ns = get_optional_attr(*child, "ns",  parent_->resolved_ns());

      const group_field_instruction* target =
        dynamic_cast<const group_field_instruction*>(this->find_type(target_ns, target_name));

      if (target == 0)
        BOOST_THROW_EXCEPTION(template_not_found_error(target_name, this->name()));

      instruction->ref_instruction(target);
    }
  }
}

void
field_builder::visit(const group_field_instruction* inst, void*)
{
  const const_instruction_ptr_t* subinstructions = inst->subinstructions();

  if (inst->subinstructions_count() == 0) {
    subinstructions = build_subfields();
  }

  group_field_instruction* instruction = new (alloc())group_field_instruction (
    parent_->num_instructions(),
    get_presence(inst),
    get_id(inst),
    get_name(alloc()),
    get_ns(inst, alloc()),
    get_dictionary(inst),
    subinstructions,
    num_instructions(),
    get_typeRef_name(element_),
    get_typeRef_ns(element_),
    inst->cpp_ns()
    );

  if (inst->subinstructions_count() == 0) {
    set_ref_instruction(instruction);
  }
  else {
    instruction->ref_instruction(inst);
  }

  parent_->add_instruction(instruction);
}

const uint32_field_instruction*
field_builder::get_length_instruction(const sequence_field_instruction* inst)
{
  const uint32_field_instruction* length_instruction = inst->length_instruction();

  const XMLElement* length_element = content_element_->FirstChildElement("length");

  if (length_element) {

    fast_xml_attributes length_attributes(length_element->FirstAttribute());

    field_op length_fop(length_instruction, length_element, alloc());

    length_instruction = new (alloc())uint32_field_instruction(
      0,
      length_fop.op_,
      get_presence(inst),
      length_attributes.get_id(length_instruction),
      length_attributes.get_name(alloc()),
      length_attributes.get_ns(length_instruction, alloc()),
      length_fop.context_,
      int_value_storage<uint32_t>(length_fop.initial_value_)
      );
  }
  else if (length_instruction->optional() != get_presence(inst))
  {
    length_instruction = new (alloc())uint32_field_instruction(
      0,
      length_instruction->field_operator(),
      get_presence(inst),
      length_instruction->id(),
      length_instruction->name(),
      length_instruction->ns(),
      length_instruction->op_context(),
      int_value_storage<uint32_t>(length_instruction->initial_value())
      );
  }
  return length_instruction;
}

void
field_builder::visit(const sequence_field_instruction* inst, void*)
{
  const const_instruction_ptr_t* subinstructions = inst->subinstructions();

  if (inst->subinstructions_count() == 0) {
    subinstructions = build_subfields();
  }

  sequence_field_instruction* instruction = new (alloc())sequence_field_instruction (
    parent_->num_instructions(),
    get_presence(inst),
    get_id(inst),
    get_name(alloc()),
    get_ns(inst, alloc()),
    get_dictionary(inst),
    subinstructions,
    num_instructions(),
    get_length_instruction(inst),
    get_typeRef_name(element_),
    get_typeRef_ns(element_),
    inst->cpp_ns()
    );

  if (inst->subinstructions_count() == 0) {
    instruction->ref_instruction(0);
  }
  else {
    instruction->ref_instruction(inst);
  }
  parent_->add_instruction(instruction);
}

void field_builder::visit(const template_instruction* inst, void*)
{
  bool reset = false;
  const XMLAttribute* reset_attr = element_.FindAttribute("scp:reset");
  if (reset_attr == 0)
    reset_attr = element_.FindAttribute("reset");

  if (reset_attr) {
    if (strcmp(reset_attr->Value(), "true") == 0 || strcmp(reset_attr->Value(), "yes") == 0)
      reset = true;
  }

  const const_instruction_ptr_t* subinstructions  = build_subfields();

  template_instruction* instruction = new (alloc())template_instruction (
    id_ ? boost::lexical_cast<uint32_t>(id_) : 0,
    string_dup(name_,       alloc()),
    string_dup(ns_,         alloc()),
    string_dup(templateNs_, alloc()),
    string_dup(dictionary_, alloc()),
    subinstructions,
    num_instructions(),
    reset,
    get_typeRef_name(element_),
    get_typeRef_ns(element_),
    inst->cpp_ns()
    );

  parent_->add_template(this->resolved_ns(), instruction);
}

void field_builder::add_template(const char*, template_instruction* inst)
{
  BOOST_THROW_EXCEPTION(fast_static_error("S1") << reason_info("template cannot be nested")
                                                << template_name_info(inst->name())
                                                << referenced_by_info(parent_->name()));
}

void field_builder::visit(const enum_field_instruction* inst, void*)
{
  field_op fop(inst, &element_, alloc());

  const char** enum_element_names = inst->elements_;
  uint64_t num_elements = inst->num_elements_;

  const char* init_value_str = 0;
  if (!fop.initial_value_.is_defined())
  {
    // if the  defined flag is false, the content value is parsed string from XML
    init_value_str = fop.initial_value_.get<const char*>();
  }

  if (enum_element_names == 0 ) {

    std::deque<const char*> names;
    const XMLElement* xml_element = element_.FirstChildElement("element");
    for (; xml_element != 0; xml_element = xml_element->NextSiblingElement("element"))
    {
      const char* name_attr = xml_element->Attribute("name");
      if (name_attr != 0) {
        if (init_value_str && std::strcmp(name_attr, init_value_str) == 0)
        {
          fop.initial_value_.set<uint64_t>(names.size());
        }
        names.push_back(string_dup(name_attr, alloc()));
      }
      else {
        throw std::runtime_error("XML element must have a name attribute");
      }
    }

    num_elements = names.size();
    enum_element_names = static_cast<const char**>(alloc().allocate(names.size()* sizeof(const char*) ));
    std::copy(names.begin(), names.end(), enum_element_names);
  }
  else if (init_value_str) {
    // In this case, the element names are already defined, but we haven't decide what the specified
    // initial value is.

    for (uint64_t i = 0; i < num_elements; ++i) {
      if (std::strcmp(enum_element_names[i], init_value_str) == 0)
      {
        fop.initial_value_.set<uint64_t>(i);
        break;
      }
    }
  }

  if (!fop.initial_value_.is_defined())
  {
    if (fop.initial_value_.get<const char*>() != 0) {
      std::string msg = "Invalid initial value for enum : ";
      throw std::runtime_error(msg + fop.initial_value_.get<const char*>());
    }
    else {
      // at this point if initial_value_ is still undefined, we should reset it to zero
      fop.initial_value_.set<uint64_t>(0);
    }
  }

  enum_field_instruction* instruction = new (alloc()) enum_field_instruction (
    parent_->num_instructions(),
    fop.op_,
    get_presence(inst),
    get_id(inst),
    get_name(alloc()),
    get_ns(inst, alloc()),
    fop.context_,
    int_value_storage<uint64_t>(fop.initial_value_),
    enum_element_names,
    num_elements,
    inst->elements_ == 0 ? 0 : inst,
    inst->cpp_ns()
    );

  parent_->add_instruction(instruction);
}

}   /* coder */

} /* mfast */
