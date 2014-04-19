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

#ifndef ENUM_INSTRUCTION_H_Y9NRM0OD
#define ENUM_INSTRUCTION_H_Y9NRM0OD

#include "int_instructions.h"

namespace mfast
{
  class MFAST_EXPORT enum_field_instruction
    : public integer_field_instruction_base
    , public referable_instruction<enum_field_instruction>
  {
  public:

    enum_field_instruction(operator_enum_t               operator_id,
                           presence_enum_t               optional,
                           uint32_t                      id,
                           const char*                   name,
                           const char*                   ns,
                           const op_context_t*           context,
                           int_value_storage<uint64_t>   initial_value,
                           const char**                  elements,
                           const uint64_t*               element_values,
                           uint64_t                      num_elements,
                           const enum_field_instruction* ref,
                           const char*                   cpp_ns,
                           instruction_tag               tag= instruction_tag())
      : integer_field_instruction_base(operator_id,
                                       field_type_enum,
                                       optional,
                                       id,
                                       name,
                                       ns,
                                       context,
                                       initial_value.storage_,
                                       tag)
      , referable_instruction<enum_field_instruction>(ref, cpp_ns)
      , elements_(elements)
      , num_elements_(num_elements)
      , element_values_(element_values)
    {
    }

    enum_field_instruction(const enum_field_instruction &other)
      : integer_field_instruction_base(other)
      , referable_instruction<enum_field_instruction>(other)
      , elements_(other.elements_)
      , num_elements_(other.num_elements_)
      , element_values_(other.element_values_)
    {
    }

    virtual void accept(field_instruction_visitor& visitor, void* context) const;
    virtual enum_field_instruction* clone(arena_allocator& alloc) const;

    const char* element_name(uint64_t v) const
    {
      if (element_values_ == 0)
        return elements_[v];
      const uint64_t* it = std::lower_bound(element_values_, element_values_+num_elements_, v);
      if (it != element_values_+num_elements_ && *it == v)
        return elements_[it-element_values_];
      return 0;
    }

    uint64_t num_elements() const
    {
      return num_elements_;
    }

    const char** elements() const
    {
      return elements_;
    }

    const uint64_t* element_values() const
    {
      return element_values_;
    }

    const char** elements_;
    uint64_t num_elements_;
    const uint64_t* element_values_;
  };

} /* mfast */


#endif /* end of include guard: ENUM_INSTRUCTION_H_Y9NRM0OD */
