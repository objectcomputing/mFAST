// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.

#pragma once
#include "codegen_base.h"
#include <vector>

class cpp_gen : public codegen_base {
public:
  cpp_gen(const char *filebase, const char *fileext = ".cpp")
      : codegen_base(filebase, fileext), hpp_fileext_(".h") {}
  void set_hpp_fileext(const char *hpp_fileext) { hpp_fileext_ = hpp_fileext; }
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
  virtual void visit(const mfast::group_field_instruction *, void *) override;
  virtual void visit(const mfast::sequence_field_instruction *,
                     void *) override;
  virtual void visit(const mfast::template_instruction *, void *) override;
  virtual void visit(const mfast::templateref_instruction *, void *) override;
  virtual void visit(const mfast::int32_vector_field_instruction *,
                     void *) override;
  virtual void visit(const mfast::uint32_vector_field_instruction *,
                     void *) override;
  virtual void visit(const mfast::int64_vector_field_instruction *,
                     void *) override;
  virtual void visit(const mfast::uint64_vector_field_instruction *,
                     void *) override;
  virtual void visit(const mfast::enum_field_instruction *, void *) override;

private:
  virtual void generate(const mfast::aggregate_view_info &info);

  std::string prefix_string() const;
  std::string cref_scope() const;
  void add_to_instruction_list(const std::string &name);
  std::string gen_op_context(const char *name,
                             const mfast::op_context_t *context);
  void gen_field(const mfast::field_instruction *inst,
                 const std::string &context, const char *cpp_type,
                 void *pIndex);
  void gen_integer(const mfast::integer_field_instruction_base *inst,
                   const char *cpp_type, const std::string &initial_value,
                   void *pIndex);
  void gen_string(const mfast::ascii_field_instruction *inst,
                  const char *charset, void *pIndex);

  void gen_int_vector(const char *cpp_type,
                      const mfast::vector_field_instruction_base *inst,
                      void *pIndex);

  void output_subinstructions();
  std::string get_subinstructions(const mfast::group_field_instruction *inst);
  bool
  need_generate_subinstructions(const mfast::group_field_instruction *inst);

  std::string hpp_fileext_;
  std::vector<std::string> subinstructions_list_;
  std::stringstream template_instructions_;
  std::vector<std::string> prefixes_;
};
