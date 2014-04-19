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
#ifndef HPP_GEN_H_EWCP8CL9
#define HPP_GEN_H_EWCP8CL9


#include <set>
#include "codegen_base.h"
#include "indented_ostream.h"

class hpp_gen
  : public codegen_base
{
public:
  hpp_gen(const char* filebase)
    : codegen_base(filebase, ".h")
  {
  }

  void set_export_symbol(const char* symbol);
  void generate(mfast::dynamic_templates_description& desc);

  virtual void visit(const mfast::int32_field_instruction*, void*);
  virtual void visit(const mfast::uint32_field_instruction*, void*);
  virtual void visit(const mfast::int64_field_instruction*, void*);
  virtual void visit(const mfast::uint64_field_instruction*, void*);
  virtual void visit(const mfast::decimal_field_instruction*, void*);
  virtual void visit(const mfast::ascii_field_instruction*, void*);
  virtual void visit(const mfast::unicode_field_instruction*, void*);
  virtual void visit(const mfast::byte_vector_field_instruction*, void*);
  virtual void visit(const mfast::int32_vector_field_instruction*, void*);
  virtual void visit(const mfast::uint32_vector_field_instruction*, void*);
  virtual void visit(const mfast::int64_vector_field_instruction*, void*);
  virtual void visit(const mfast::uint64_vector_field_instruction*, void*);
  virtual void visit(const mfast::group_field_instruction*, void*);
  virtual void visit(const mfast::sequence_field_instruction*, void*);
  virtual void visit(const mfast::template_instruction*, void*);
  virtual void visit(const mfast::templateref_instruction*, void*);
  virtual void visit(const mfast::enum_field_instruction*, void*);

private:
  void gen_primitive (const char* cpp_type, const mfast::field_instruction* inst);
  virtual void traverse(const mfast::group_field_instruction* inst, const char* name_suffix);
  virtual void generate(const mfast::aggregate_view_info& info);
  void gen_sequence_typedef(const mfast::sequence_field_instruction* inst, const std::string& name);

  typedef indented_stringstream ind_stream;
  std::set<std::string> dependency_;
  ind_stream header_cref_;
  ind_stream header_mref_;
  std::stringstream content_;
  std::string export_symbol_;
  std::string export_symbol_uppercase_;

};

#endif /* end of include guard: HPP_GEN_H_EWCP8CL9 */
