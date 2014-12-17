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

#ifndef FIELD_INSTRUCTION_H_PMKVDZOC
#define FIELD_INSTRUCTION_H_PMKVDZOC

#ifdef _MSC_VER
#pragma warning(disable: 4275) // non dll-interface class used as base for dll-interface class
#endif //_MSC_VER

#include "instructions/int_instructions.h"
#include "instructions/enum_instruction.h"
#include "instructions/decimal_instruction.h"
#include "instructions/string_instructions.h"
#include "instructions/byte_vector_instruction.h"
#include "instructions/int_vector_instructions.h"
#include "instructions/group_instruction.h"
#include "instructions/sequence_instruction.h"
#include "instructions/template_instruction.h"
#include "instructions/templateref_instruction.h"
#include "instructions/templates_description.h"

namespace mfast {

  class MFAST_EXPORT field_instruction_visitor
  {
  public:
    virtual void visit(const int32_field_instruction*, void*)=0;
    virtual void visit(const uint32_field_instruction*, void*)=0;
    virtual void visit(const int64_field_instruction*, void*)=0;
    virtual void visit(const uint64_field_instruction*, void*)=0;
    virtual void visit(const decimal_field_instruction*, void*)=0;
    virtual void visit(const ascii_field_instruction*, void*)=0;
    virtual void visit(const unicode_field_instruction*, void*)=0;
    virtual void visit(const byte_vector_field_instruction*, void*)=0;
    virtual void visit(const group_field_instruction*, void*)=0;
    virtual void visit(const sequence_field_instruction*, void*)=0;
    virtual void visit(const template_instruction*, void*)=0;
    virtual void visit(const templateref_instruction*, void*)=0;

    virtual void visit(const int32_vector_field_instruction*, void*)=0;
    virtual void visit(const uint32_vector_field_instruction*, void*)=0;
    virtual void visit(const int64_vector_field_instruction*, void*)=0;
    virtual void visit(const uint64_vector_field_instruction*, void*)=0;

    virtual void visit(const enum_field_instruction* inst, void* data)=0;

  };


///////////////////////////////////////////////////////////////////////////////////

  template <typename T>
  void int_field_instruction<T>::accept(field_instruction_visitor& visitor, void* context) const
  {
    visitor.visit(this, context);
  }

  template <typename T>
  void vector_field_instruction<T>::accept(field_instruction_visitor& visitor, void* context) const
  {
    visitor.visit(this, context);
  }

///////////////////////////////////////////////////////

#ifdef _MSC_VER

  template class MFAST_EXPORT int_field_instruction<int32_t>;
  template class MFAST_EXPORT int_field_instruction<uint32_t>;
  template class MFAST_EXPORT int_field_instruction<int64_t>;
  template class MFAST_EXPORT int_field_instruction<uint64_t>;
#endif

} // namespace mfast
#endif /* end of include guard: FIELD_INSTRUCTION_H_PMKVDZOC */
