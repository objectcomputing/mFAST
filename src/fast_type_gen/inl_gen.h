// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.
#pragma once

#include "codegen_base.h"

class inl_gen : public codegen_base {
public:
  inl_gen(const char *filebase, const char *fileext = ".inl") : codegen_base(filebase, fileext) {}
  void generate(mfast::dynamic_templates_description &desc);

  virtual void visit(const mfast::int32_field_instruction *, void *) override;
  virtual void visit(const mfast::uint32_field_instruction *, void *) override;
  virtual void visit(const mfast::int64_field_instruction *, void *) override;
  virtual void visit(const mfast::uint64_field_instruction *, void *) override;
  virtual void visit(const mfast::decimal_field_instruction *, void *) override;
  virtual void visit(const mfast::ascii_field_instruction *, void *) override;
  virtual void visit(const mfast::unicode_field_instruction *, void *) override;
  virtual void visit(const mfast::byte_vector_field_instruction *,
                     void *) override;
  virtual void visit(const mfast::int32_vector_field_instruction *,
                     void *) override;
  virtual void visit(const mfast::uint32_vector_field_instruction *,
                     void *) override;
  virtual void visit(const mfast::int64_vector_field_instruction *,
                     void *) override;
  virtual void visit(const mfast::uint64_vector_field_instruction *,
                     void *) override;
  virtual void visit(const mfast::group_field_instruction *, void *) override;
  virtual void visit(const mfast::sequence_field_instruction *,
                     void *) override;
  virtual void visit(const mfast::template_instruction *, void *) override;
  virtual void visit(const mfast::templateref_instruction *, void *) override;
  virtual void visit(const mfast::enum_field_instruction *, void *) override;

private:
  virtual void traverse(const mfast::group_field_instruction *inst,
                        const char *name_suffix) override;
  virtual void generate(const mfast::aggregate_view_info &info);

  void gen_primitive(const char *cpp_type, const mfast::field_instruction *inst,
                     void *pIndex);

  void gen_accessors(const mfast::field_instruction *inst,
                     const std::string &name, const std::string &cref_type_name,
                     const std::string &mref_type_name, void *pIndex);

  std::stringstream mref_scope_;
};
