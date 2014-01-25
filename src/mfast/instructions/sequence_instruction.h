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

#ifndef SEQUENCE_INSTRUCTION_H_QKBH3IK2
#define SEQUENCE_INSTRUCTION_H_QKBH3IK2
#include "int_instructions.h"
#include "group_instruction.h"

namespace mfast
{
  class MFAST_EXPORT sequence_field_instruction
    : public group_field_instruction
  {
  public:
    sequence_field_instruction(uint16_t                          field_index,
                               presence_enum_t                   optional,
                               uint32_t                          id,
                               const char*                       name,
                               const char*                       ns,
                               const char*                       dictionary,
                               const const_instruction_ptr_t*    subinstructions,
                               uint32_t                          subinstructions_count,
                               const uint32_field_instruction*   sequence_length_instruction,
                               const char*                       typeref_name="",
                               const char*                       typeref_ns="",
                               const char*                       cpp_ns="",
                               const group_field_instruction*    element_instruction=0,
                               const sequence_field_instruction* ref_instruction=0);


    virtual void construct_value(value_storage& storage,
                                 allocator*     alloc) const;
    virtual void destruct_value(value_storage& storage,
                                allocator*     alloc) const;


    virtual void construct_value(value_storage& storage,
                                 value_storage* fields_storage,
                                 allocator*     alloc,
                                 bool           construct_subfields) const;

    void construct_sequence_elements(value_storage& storage,
                                     std::size_t    start,
                                     std::size_t    length,
                                     allocator*     alloc) const;
    void destruct_sequence_elements(value_storage& storage,
                                    std::size_t    start,
                                    std::size_t    length,
                                    allocator*     alloc) const;

    // perform deep copy
    virtual void copy_construct_value(const value_storage& src,
                                      value_storage&       dest,
                                      allocator*           alloc,
                                      value_storage*       fields_storage=0) const;

    virtual void accept(field_instruction_visitor&, void*) const;
    const uint32_field_instruction* length_instruction() const
    {
      return sequence_length_instruction_;
    }

    const uint32_field_instruction*& length_instruction()
    {
      return sequence_length_instruction_;
    }

    virtual sequence_field_instruction* clone(arena_allocator& alloc) const;

    const group_field_instruction* element_instruction() const
    {
      return element_instruction_;
    }

    void element_instruction(const group_field_instruction* i)
    {
      element_instruction_ = i;
    }

  private:

    friend class dictionary_builder;
    const uint32_field_instruction* sequence_length_instruction_;
    const group_field_instruction* element_instruction_;
  };

  template <typename T>
  class sequence_instruction_ex
    : public sequence_field_instruction
  {
  public:
    sequence_instruction_ex(uint16_t                          field_index,
                            presence_enum_t                   optional,
                            uint32_t                          id,
                            const char*                       name,
                            const char*                       ns,
                            const char*                       dictionary,
                            const const_instruction_ptr_t*    subinstructions,
                            uint32_t                          subinstructions_count,
                            uint32_field_instruction*         sequence_length_instruction,
                            const char*                       typeref_name,
                            const char*                       typeref_ns,
                            const char*                       cpp_ns,
                            const group_field_instruction*    element_instruction,
                            const sequence_field_instruction* ref_instruction)
      : sequence_field_instruction(field_index, optional, id, name, ns, dictionary,
                                   subinstructions, subinstructions_count, sequence_length_instruction,
                                   typeref_name, typeref_ns, cpp_ns,
                                   element_instruction, ref_instruction)
    {
    }

    virtual sequence_instruction_ex<T>* clone(arena_allocator& alloc) const
    {
      return new (alloc) sequence_instruction_ex<T>(*this);
    }

  };

} /* mfast */

#endif /* end of include guard: SEQUENCE_INSTRUCTION_H_QKBH3IK2 */
