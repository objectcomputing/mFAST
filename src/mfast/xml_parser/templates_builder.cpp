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

#include "templates_builder.h"
#include "mfast/boolean_ref.h"
#include "view_info_builder.h"
#include "mfast/exceptions.h"

using namespace tinyxml2;


//////////////////////////////////////////////////////////////////////////////////////
namespace mfast
{
  namespace xml_parser
  {

    static const uint32_field_instruction length_instruction_prototype(operator_none,presence_mandatory,0,"__length__","",0, int_value_storage<uint32_t>());

    templates_builder::templates_builder(dynamic_templates_description* definition,
                                         const char*                    cpp_ns,
                                         template_registry*             registry)
      : field_builder_base(registry->impl_, &this->member)
      , definition_(definition)
      , cpp_ns_(string_dup(cpp_ns, this->alloc()))
      , template_instruction_prototype_(0,0,"","","",instructions_view_t(0,0),0,0,0,cpp_ns_)
      , group_field_instruction_prototype_(presence_mandatory,0,0,"","",instructions_view_t(0,0), "", "", cpp_ns_)
      , sequence_field_instruction_prototype_(presence_mandatory,0,0,"","",instructions_view_t(0,0),0,0,&length_instruction_prototype, "", "", cpp_ns_)
      , enum_field_instruction_prototype_(operator_none,presence_mandatory,0,0,"",0,0,0,0,0,0,cpp_ns_)
    {
      static const int32_field_instruction int32_field_instruction_prototype(operator_none,presence_mandatory,0,0,"",0, int_value_storage<int32_t>());
      this->member["int32"] = &int32_field_instruction_prototype;

      static const uint32_field_instruction uint32_field_instruction_prototype(operator_none,presence_mandatory,0,0,"",0, int_value_storage<uint32_t>());
      this->member["uInt32"] = &uint32_field_instruction_prototype;

      static const int64_field_instruction int64_field_instruction_prototype(operator_none,presence_mandatory,0,0,"",0, int_value_storage<int64_t>());
      this->member["int64"] = &int64_field_instruction_prototype;

      static const uint64_field_instruction uint64_field_instruction_prototype(operator_none,presence_mandatory,0,0,"",0, int_value_storage<uint64_t>());
      this->member["uInt64"] = &uint64_field_instruction_prototype;

      static const decimal_field_instruction decimal_field_instruction_prototype(operator_none,presence_mandatory,0,0,"",0, decimal_value_storage());
      this->member["decimal"] = &decimal_field_instruction_prototype;

      static const ascii_field_instruction ascii_field_instruction_prototype(operator_none,presence_mandatory,0,0,"",0, string_value_storage());
      this->member["string"] = &ascii_field_instruction_prototype;

      static const byte_vector_field_instruction byte_vector_field_instruction_prototype(operator_none,presence_mandatory,0,0,"",0, string_value_storage(), 0, "", "");
      this->member["byteVector"] = &byte_vector_field_instruction_prototype;

      static const int32_vector_field_instruction int32_vector_field_instruction_prototype(presence_mandatory,0,0,"");
      this->member["int32Vector"] = &int32_vector_field_instruction_prototype;

      static const uint32_vector_field_instruction uint32_vector_field_instruction_prototype(presence_mandatory,0,0,"");
      this->member["uInt32Vector"] = &uint32_vector_field_instruction_prototype;

      static const int64_vector_field_instruction int64_vector_field_instruction_prototype(presence_mandatory,0,0,"");
      this->member["int64Vector"] = &int64_vector_field_instruction_prototype;

      static const uint64_vector_field_instruction uint64_vector_field_instruction_prototype(presence_mandatory,0,0,"");
      this->member["uInt64Vector"] = &uint64_vector_field_instruction_prototype;

      this->member["group"] = &group_field_instruction_prototype_;
      this->member["sequence"] = &sequence_field_instruction_prototype_;
      this->member["template"] = &template_instruction_prototype_;
      this->member["boolean"] = mfast::boolean::instruction();
      this->member["enum"] = &enum_field_instruction_prototype_;
    }

    bool
    templates_builder::VisitEnter (const XMLElement & element, const XMLAttribute*)
    {
      const char* element_name = element.Name();

      if (std::strcmp(element_name, "templates") == 0 ) {
        definition_->ns_ = string_dup(get_optional_attr(element, "ns", ""), alloc());

        resolved_ns_= string_dup(get_optional_attr(element, "templateNs", ""), alloc());
        definition_->template_ns_ = resolved_ns_;
        definition_->dictionary_ = string_dup(get_optional_attr(element, "dictionary", ""), alloc());
        return true;
      }
      else if (strcmp(element_name, "define") == 0) {
        const char* name =  get_optional_attr(element, "name", 0);
        const XMLElement* elem = element.FirstChildElement();
        if (name && elem) {
          field_builder builder(this, *elem, name);
          builder.build();
        }
      }
      else if (strcmp(element_name, "template") == 0) {
        field_builder builder(this, element);
        builder.build();
      }
      else if (strcmp(element_name, "view") == 0) {
        view_info_builder builder(alloc());
        const group_field_instruction* inst = dynamic_cast<const group_field_instruction*>(this->find_type(
                                                                                             get_optional_attr(element,"ns",        resolved_ns_),
                                                                                             get_optional_attr(element,"reference", "")));

        if (inst == 0)
          BOOST_THROW_EXCEPTION(fast_static_error("Invalid view specification"));
        definition_->view_infos_.push_back( builder.build(element, inst) );
      }
      return false;
    }

    bool
    templates_builder::VisitExit (const XMLElement & element)
    {
      if (std::strcmp(element.Name(), "templates") == 0 ) {
        typedef const template_instruction* const_template_instruction_ptr_t;

        definition_->instructions_ = new (alloc())const_template_instruction_ptr_t[this->num_instructions()];
        std::copy(templates_.begin(), templates_.end(), definition_->instructions_);
        definition_->instructions_count_ = static_cast<uint32_t>(templates_.size());
      }
      return true;
    }

    std::size_t templates_builder::num_instructions() const
    {
      return local_types()->size();
    }

    void templates_builder::add_instruction(const field_instruction* inst)
    {
      member[inst->name()] = inst;

      if (inst->field_type() >= field_type_sequence) {
        definition_->defined_type_instructions_.push_back(inst);
        const char* ns = inst->ns();
        if (ns == 0 || ns[0] == '\0')
          ns = resolved_ns_;
        registry()->add(ns, inst);
      }
    }

    void templates_builder::add_template(const char* ns, template_instruction* inst)
    {
      templates_.push_back(inst);
      registry()->add(ns, inst);
    }

/////////////////////////////////////////////////////////////////////////////////
  } /* coder */


  template_registry::template_registry()
    : impl_(new xml_parser::template_registry_impl)
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

  arena_allocator* template_registry::alloc()
  {
    return &(this->impl_->alloc_);
  }

  dynamic_templates_description::dynamic_templates_description(const char*        xml_content,
                                                               const char*        cpp_ns,
                                                               template_registry* registry)
  {
    XMLDocument document;

	// Get document length
	size_t len = strlen(xml_content);

	// Make sure that the document ends in ">"
	while (xml_content[len] != '>')
	{
		--len;
	}
	++len;

    if (document.Parse(xml_content, len) == 0) {
      xml_parser::templates_builder builder(this, cpp_ns, registry);
      document.Accept(&builder);
    }
    else {
      BOOST_THROW_EXCEPTION(std::runtime_error("XML parse error"));
    }
  }

  const std::deque<const field_instruction*>&
  dynamic_templates_description::defined_type_instructions() const
  {
    return defined_type_instructions_;
  }

} /* mfast */
