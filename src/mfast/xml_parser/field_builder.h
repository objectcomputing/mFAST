// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.
#pragma once

#include "field_builder_base.h"
#include "fast_xml_attributes.h"
namespace mfast {
namespace xml_parser {
typedef std::deque<const field_instruction *> instruction_list_t;

class field_builder : public fast_xml_attributes,
                      public field_builder_base,
                      public field_instruction_visitor {
protected:
  instruction_list_t instructions_;

  const char *field_type_name_;
  const XMLElement &element_;
  const XMLElement *content_element_;
  field_builder_base *parent_;

public:
  field_builder(field_builder_base *parent, const XMLElement &element);

  // used for build define element
  field_builder(field_builder_base *parent, const XMLElement &element,
                const char *type_name);

  void build();

  const char *resolve_field_type(const XMLElement &element);

  const field_instruction *find_prototype(const char *type_name);

  const char *field_type_name() const { return field_type_name_; }
  virtual const char *name() const override;
  virtual std::size_t num_instructions() const override;
  virtual void add_instruction(const field_instruction *) override;
  virtual void add_template(const char *ns,
                            template_instruction *inst) override;

protected:
  template <typename Instruction>
  uint32_t get_length_id(const Instruction *inst,
                         const fast_xml_attributes &length_attrs) const {
    return length_attrs.id_ ? static_cast<uint32_t>(std::stoi(length_attrs.id_))
                            : inst->length_id();
  }

  template <typename Instruction>
  const char *get_length_name(const Instruction *inst,
                              const fast_xml_attributes &length_attrs) const {
    return length_attrs.name_ ? string_dup(length_attrs.name_, alloc())
                              : inst->length_name();
  }

  template <typename Instruction>
  const char *get_length_ns(const Instruction *inst,
                            const fast_xml_attributes &length_attrs) const {
    return length_attrs.ns_ ? string_dup(length_attrs.ns_, alloc())
                            : inst->length_ns();
  }

  template <typename Instruction>
  const char *get_dictionary(const Instruction *inst) const {
    return dictionary_ ? string_dup(dictionary_, alloc()) : inst->dictionary();
  }

  virtual void visit(const int32_field_instruction *, void *) override;
  virtual void visit(const uint32_field_instruction *, void *) override;
  virtual void visit(const int64_field_instruction *, void *) override;
  virtual void visit(const uint64_field_instruction *, void *) override;
  template <typename IntType>
  void build_integer(const int_field_instruction<IntType> *inst);

  virtual void visit(const decimal_field_instruction *, void *) override;

  virtual void visit(const ascii_field_instruction *, void *) override;

  virtual void visit(const unicode_field_instruction *, void *) override;
  virtual void visit(const byte_vector_field_instruction *, void *) override;

  virtual void visit(const group_field_instruction *, void *) override;
  virtual void visit(const sequence_field_instruction *, void *) override;
  virtual void visit(const template_instruction *, void *) override;
  virtual void visit(const templateref_instruction *, void *) override;

  virtual void visit(const int32_vector_field_instruction *, void *) override;
  virtual void visit(const uint32_vector_field_instruction *, void *) override;
  virtual void visit(const int64_vector_field_instruction *, void *) override;
  virtual void visit(const uint64_vector_field_instruction *, void *) override;

  virtual void visit(const enum_field_instruction *, void *) override;

  instructions_view_t build_subfields();
  const group_field_instruction *get_sole_templateref();
  const uint32_field_instruction *
  get_length_instruction(const sequence_field_instruction *inst);

  instruction_tag parse_tag(const field_instruction *inst);
};

} /* xml_parser */

} /* mfast */
