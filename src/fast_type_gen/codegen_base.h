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
#ifndef CODEGEN_BASE_H_JZ0IUHN2
#define CODEGEN_BASE_H_JZ0IUHN2

#include <string>
#include <boost/exception/all.hpp>
#include <mfast.h>
#include <mfast/coder/dynamic_templates_description.h>
#include <fstream>

class file_open_error
  : public virtual boost::exception, public virtual std::exception
{
  public:
    file_open_error()
    {
    }

    file_open_error(const std::string& filename)
    {
      *this << boost::errinfo_file_name(filename) << boost::errinfo_errno(errno);
    }
};

class codegen_base
  : public mfast::field_instruction_visitor
{
protected:
  std::string filebase_;
  std::ofstream out_;
  std::stringstream cref_scope_;
public:
  codegen_base(const char* filebase, const char* fileext);
protected:
  void traverse(mfast::dynamic_templates_description& desc);
  virtual void traverse(const mfast::group_field_instruction* inst, const char* name_suffix="");

  bool contains_only_templateRef(const mfast::group_field_instruction* inst);

  void reset_scope(std::stringstream& strm, const std::string& str);
  std::string cpp_name(const mfast::field_instruction* inst) const;
  std::string cpp_name(const char* n) const;

  template <typename ReferableInstruction>
  std::string ref_instruction_name(const ReferableInstruction* inst) const
  {
    if (inst->ref_instruction()->cpp_ns() && inst->ref_instruction()->cpp_ns()[0] != 0)
     return std::string(inst->ref_instruction()->cpp_ns()) + "::" + inst->ref_instruction()->name();
    else
      return inst->ref_instruction()->name();
  }
};


#endif /* end of include guard: CODEGEN_BASE_H_JZ0IUHN2 */
