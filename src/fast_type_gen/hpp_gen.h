// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information. 
#pragma once

#include <set>
#include "codegen_base.h"
#include "indented_ostream.h"

class hpp_gen : public codegen_base {
public:
  hpp_gen(const char *filebase, const char *fileext = ".h")
      : codegen_base(filebase, fileext), hpp_fileext_(fileext), inl_fileext_(".inl") {}
  void set_export_symbol(const char *symbol);
  void set_inl_fileext(const char *inl_fileext) { inl_fileext_ = inl_fileext; }
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
  void gen_primitive(const char *cpp_type,
                     const mfast::field_instruction *inst);
  virtual void traverse(const mfast::group_field_instruction *inst,
                        const char *name_suffix) override;
  virtual void generate(const mfast::aggregate_view_info &info);
  void gen_sequence_typedef(const mfast::sequence_field_instruction *inst,
                            const std::string &name);

  typedef indented_stringstream ind_stream;
  std::string hpp_fileext_;
  std::string inl_fileext_;
  std::set<std::string> dependency_;
  ind_stream header_cref_;
  ind_stream header_mref_;
  std::stringstream content_;
  std::string export_symbol_;
  std::string export_symbol_uppercase_;
};
