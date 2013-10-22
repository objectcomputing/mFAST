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
#include "codegen_base.h"

codegen_base::codegen_base(const char* filebase, const char* fileext)
  : filebase_(filebase)
  , out_((filebase_+fileext).c_str(), std::ofstream::trunc)
{
  if (!out_.is_open()) {
    filebase_ += fileext;
    throw file_open_error(filebase_);
  }
}

void codegen_base::traverse(mfast::dynamic_templates_description& desc)
{
  for (size_t i = 0; i < desc.size(); ++i) {
    desc[i]->accept(*this, 0);
  }
}

void codegen_base::traverse(const mfast::group_field_instruction* inst, const char* name_suffix)
{
  std::string saved_cref_scope = cref_scope_.str();
  cref_scope_ << inst->name() << name_suffix << "_cref::";
  for (std::size_t i = 0; i < inst->subinstructions_count(); ++i)
  {
    inst->subinstruction(i)->accept(*this, 0);
  }
  reset_scope(cref_scope_, saved_cref_scope);
}

void codegen_base::reset_scope(std::stringstream& strm, const std::string& str)
{
  strm.clear();
  strm.str(str);
  strm.seekp(str.size());
}

bool codegen_base::contains_only_templateRef(const mfast::group_field_instruction* inst)
{
  return inst->ref_template() != 0 ||
         (inst->subinstructions_count() == 1 && inst->subinstruction(0)->field_type() == mfast::field_type_templateref);
}
