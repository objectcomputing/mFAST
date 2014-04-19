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
#include "codegen_base.h"
#include <cctype>
#include <boost/foreach.hpp>

codegen_base::codegen_base(const char* filebase, const char* fileext)
  : filebase_(filebase)
  , cpp_ns_(filebase)
  , out_((filebase_+fileext).c_str(), std::ofstream::trunc)
{
  if (!out_.is_open()) {
    filebase_ += fileext;
    throw file_open_error(filebase_);
  }
}

inline bool
codegen_base::dont_generate(const mfast::field_instruction* /*inst*/) const
{
  // return std::strncmp("mfast:", inst->name(), 6) == 0;
  return false;
}

void codegen_base::traverse(mfast::dynamic_templates_description& desc)
{
  BOOST_FOREACH(const mfast::field_instruction* inst, desc.defined_type_instructions())
  {
    // we use the second parameter to identify wether the instruction is nested. If the
    // second parameter is not 0, it is nested inside another composite types.
     if (!dont_generate(inst))
       inst->accept(*this, 0);
  }

  for (size_t i = 0; i < desc.size(); ++i)
  {
    desc[i]->accept(*this, 0);
  }
}

void codegen_base::traverse(const mfast::group_field_instruction* inst, const char* name_suffix)
{
  std::string saved_cref_scope = cref_scope_.str();
  cref_scope_ << cpp_name(inst) << name_suffix << "_cref::";

  for (std::size_t i = 0; i < inst->subinstructions().size(); ++i)
  {
    const mfast::field_instruction* subinst = inst->subinstruction(i);
    // we use the second parameter to identify the index of the instruction. If the
    // second parameter is not 0, it is nested inside another composite types.
    subinst->accept(*this, &i);
  }
  reset_scope(cref_scope_, saved_cref_scope);
}

void codegen_base::reset_scope(std::stringstream& strm, const std::string& str)
{
  strm.str("");
  strm << str;
}

std::string
codegen_base::cpp_name(const mfast::field_instruction* inst) const
{
  return cpp_name( inst->name() );
}

std::string codegen_base::cpp_name(const char* name) const
{
  std::string result;
  if (!std::isalpha(name[0]))
    result = "_";
  while (*name != '\x0') {
    char c = *name++;
    if (isalnum(c))
      result += c;
    else
      result += '_';
  }
  return result;
}

bool codegen_base::is_const_field(const mfast::field_instruction* inst) const
{
  if (inst->field_operator() != mfast::operator_constant )
    return false;
  if (inst->field_type() == mfast::field_type_exponent) {
    const mfast::decimal_field_instruction* the_inst = static_cast<const mfast::decimal_field_instruction*>(inst);
    if (the_inst->mantissa_instruction() ==0 || the_inst->mantissa_instruction()->field_operator() != mfast::operator_constant)
      return false;
  }
  return true;
}

using namespace mfast;

class type_name_finder
  : public field_instruction_visitor
{
public:
  std::string name_;
  const char* caller_cpp_ns_;
  std::set<std::string>* dependency_;

  type_name_finder(const char*            caller_cpp_ns,
                   std::set<std::string>* dependency)
    : caller_cpp_ns_(caller_cpp_ns)
    , dependency_(dependency)
  {
  }

  virtual void visit(const int32_field_instruction*, void*)
  {
    name_ = "mfast::int32";
  }

  virtual void visit(const uint32_field_instruction*, void*)
  {
    name_ = "mfast::uint32";
  }

  virtual void visit(const int64_field_instruction*, void*)
  {
    name_ = "mfast::uint64";
  }

  virtual void visit(const uint64_field_instruction*, void*)
  {
    name_ = "mfast::int64";
  }

  virtual void visit(const decimal_field_instruction*, void*)
  {
    name_ = "mfast::decimal";
  }

  virtual void visit(const ascii_field_instruction*, void*)
  {
    name_ = "mfast::ascii_string";
  }

  virtual void visit(const unicode_field_instruction*, void*)
  {
    name_ = "mfast::unicode_string";
  }

  virtual void visit(const byte_vector_field_instruction*, void*)
  {
    name_ = "mfast::byte_vector";
  }

  virtual void visit(const group_field_instruction* inst, void* pIndex)
  {
    if (inst->ref_instruction()) {
      inst->ref_instruction()->accept(*this, pIndex);
    }
    else if (inst->cpp_ns()==0 || inst->cpp_ns()[0] == 0 ||
             strcmp(caller_cpp_ns_, inst->cpp_ns()) ==0)
    {
      name_ = inst->name();
    }
    else {
      name_ = std::string(inst->cpp_ns()) + "::" + inst->name();
      if (dependency_)
        dependency_->insert(inst->cpp_ns());
    }

  }

  virtual void visit(const sequence_field_instruction* inst, void*)
  {
    this->visit(static_cast<const group_field_instruction*>(inst), 0);
  }

  virtual void visit(const template_instruction* inst, void*)
  {
    this->visit(static_cast<const group_field_instruction*>(inst), 0);
  }

  virtual void visit(const templateref_instruction*, void*)
  {
    name_ = "mfast::nested_message";
  }

  virtual void visit(const int32_vector_field_instruction*, void*)
  {
    name_ = "mfast::int32_vector";
  }

  virtual void visit(const uint32_vector_field_instruction*, void*)
  {
    name_ = "mfast::uint32_vector";
  }

  virtual void visit(const int64_vector_field_instruction*, void*)
  {
    name_ = "mfast::int64_vector";
  }

  virtual void visit(const uint64_vector_field_instruction*, void*)
  {
    name_ = "mfast::uint64_vector";
  }

  virtual void visit(const enum_field_instruction* inst, void* pIndex)
  {
    if (inst->ref_instruction()) {
      inst->ref_instruction()->accept(*this, pIndex);
    }
    else if (inst->cpp_ns()==0 || inst->cpp_ns()[0] == 0 ||
             strcmp(caller_cpp_ns_, inst->cpp_ns()) ==0)
    {
      name_ = inst->name();
    }
    else {
      name_ = std::string(inst->cpp_ns()) + "::" + inst->name();
      if (dependency_)
        dependency_->insert(inst->cpp_ns());
    }

  }

};

std::string
codegen_base::cpp_type_of(const mfast::field_instruction* inst,
                          std::set<std::string>*          dependency) const
{
  type_name_finder finder(cpp_ns_.c_str(), dependency);
  inst->accept(finder, 0);
  return finder.name_;
}

bool codegen_base::contains_only_templateref(const mfast::group_field_instruction* inst) const
{
  return inst->subinstructions().size() == 1 && inst->subinstruction(0)->field_type() == mfast::field_type_templateref;
}

const mfast::field_instruction*
codegen_base::get_element_instruction(const mfast::sequence_field_instruction* inst) const
{
  if (inst->element_instruction())
    return inst->element_instruction();
  if (inst->subinstructions().size() == 1 && inst->subinstruction(0)->name()[0] == 0 )
    return inst->subinstruction(0);
  return 0;
}


