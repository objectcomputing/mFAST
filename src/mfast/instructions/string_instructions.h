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

#ifndef STRING_INSTRUCTIONS_H_A0SJB1X7
#define STRING_INSTRUCTIONS_H_A0SJB1X7

#include "vector_instruction_base.h"

namespace mfast
{
  class MFAST_EXPORT ascii_field_instruction
    : public vector_field_instruction_base
  {
  public:
    ascii_field_instruction(operator_enum_t      operator_id,
                            presence_enum_t      optional,
                            uint32_t             id,
                            const char*          name,
                            const char*          ns,
                            const op_context_t*  context,
                            string_value_storage initial_value,
                            instruction_tag      tag = instruction_tag(),
                            field_type_enum_t    field_type = field_type_ascii_string);

    ascii_field_instruction(const ascii_field_instruction& other);

    virtual void construct_value(value_storage& storage,
                                 allocator*     alloc) const;
    // perform deep copy
    virtual void copy_construct_value(const value_storage& src,
                                      value_storage&       dest,
                                      allocator*           alloc,
                                      value_storage*       fields_storage=0) const;

    virtual void accept(field_instruction_visitor& visitor, void* context) const;
    virtual ascii_field_instruction* clone(arena_allocator& alloc) const;

    value_storage& prev_value()
    {
      return *prev_value_;
    }

    const value_storage& prev_value() const
    {
      return *prev_value_;
    }

    const op_context_t* op_context() const
    {
      return op_context_;
    }

    void op_context(const op_context_t* v)
    {
      op_context_ = v;
    }

    const value_storage& initial_value() const
    {
      return initial_value_;
    }

    const value_storage& initial_or_default_value() const
    {
      return *initial_or_default_value_;
    }

    void initial_value(const value_storage& v);

    static const ascii_field_instruction* default_instruction();

  protected:

    virtual void update_invariant()
    {
      field_instruction::update_invariant();
      mandatory_no_initial_value_ = !optional() && initial_value_.is_empty();
    }

    friend class dictionary_builder;
    const op_context_t* op_context_;
    value_storage initial_value_;
    value_storage* prev_value_;
    value_storage prev_storage_;
    const value_storage* initial_or_default_value_;
    static const value_storage default_value_;
  };


  class MFAST_EXPORT unicode_field_instruction
    : public ascii_field_instruction
  {
  public:
    unicode_field_instruction(operator_enum_t      operator_id,
                              presence_enum_t      optional,
                              uint32_t             id,
                              const char*          name,
                              const char*          ns,
                              const op_context_t*  context,
                              string_value_storage initial_value,// = string_value_storage(),
                              uint32_t             length_id ,//= 0,
                              const char*          length_name ,//= "",
                              const char*          length_ns ,//= "",
                              instruction_tag      tag = instruction_tag(),
                              field_type_enum_t    field_type = field_type_unicode_string)
      :  ascii_field_instruction(operator_id,
                                 optional,
                                 id, name, ns, context,
                                 initial_value,
                                 tag,
                                 field_type)
      , length_id_(length_id)
      , length_name_(length_name)
      , length_ns_(length_ns)
    {
    }

    unicode_field_instruction(const unicode_field_instruction& other)
      : ascii_field_instruction(other)
      , length_id_(other.length_id_)
      , length_name_(other.length_name_)
      , length_ns_(other.length_ns_)
    {
    }

    virtual void accept(field_instruction_visitor& visitor, void* context) const;
    virtual unicode_field_instruction* clone(arena_allocator& alloc) const;

    uint32_t length_id() const
    {
      return length_id_;
    }

    const char* length_name() const
    {
      return length_name_;
    }

    const char* length_ns() const
    {
      return length_ns_;
    }

    void length_id(uint32_t v)
    {
      length_id_ = v;
    }

    void length_name(const char* v)
    {
      length_name_ = v;
    }

    void length_ns(const char* v)
    {
      length_ns_ = v;
    }

    static const unicode_field_instruction* default_instruction();

  protected:
    uint32_t length_id_;
    const char* length_name_;
    const char* length_ns_;
  };

} /* mfast */
#endif /* end of include guard: STRING_INSTRUCTIONS_H_A0SJB1X7 */
