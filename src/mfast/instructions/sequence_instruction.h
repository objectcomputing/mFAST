// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.

#pragma once

#include "int_instructions.h"
#include "group_instruction.h"

namespace mfast {
class MFAST_EXPORT sequence_field_instruction : public group_field_instruction {
public:
  sequence_field_instruction(
      presence_enum_t optional, uint32_t id, const char *name, const char *ns,
      const char *dictionary, instructions_view_t subinstructions,
      const group_field_instruction *element_instruction,
      const group_field_instruction *ref_instruction,
      const uint32_field_instruction *sequence_length_instruction,
      const char *typeref_name, const char *typeref_ns, const char *cpp_ns,
      instruction_tag tag = instruction_tag());

  virtual void construct_value(value_storage &storage,
                               allocator *alloc) const override;
  virtual void destruct_value(value_storage &storage,
                              allocator *alloc) const override;

  virtual void construct_value(value_storage &storage,
                               value_storage *fields_storage, allocator *alloc,
                               bool construct_subfields) const override;

  void construct_sequence_elements(value_storage &storage, std::size_t start,
                                   std::size_t length, allocator *alloc) const;
  void destruct_sequence_elements(value_storage &storage, std::size_t start,
                                  std::size_t length, allocator *alloc) const;

  // perform deep copy
  virtual void
  copy_construct_value(const value_storage &src, value_storage &dest,
                       allocator *alloc,
                       value_storage *fields_storage = nullptr) const override;

  virtual void accept(field_instruction_visitor &, void *) const override;
  const uint32_field_instruction *length_instruction() const {
    return sequence_length_instruction_;
  }

  const uint32_field_instruction *&length_instruction() {
    return sequence_length_instruction_;
  }
  virtual sequence_field_instruction *
  clone(arena_allocator &alloc) const override;

  const group_field_instruction *element_instruction() const {
    return element_instruction_;
  }
  void element_instruction(const group_field_instruction *i) {
    element_instruction_ = i;
  }
  int properties() const {
    // This member function is used to query the properties of sequence element.
    // Becuase the sequence element can never be optional, the presence bit
    // should never be
    // encoded.

    return /* (optional() ? 1 : 0) | */ static_cast<int>(segment_pmap_size()
                                                         << 1);
  }

private:
  friend class dictionary_builder;
  const uint32_field_instruction *sequence_length_instruction_;
  const group_field_instruction *element_instruction_;
};

template <typename T>
class sequence_instruction_ex : public sequence_field_instruction {
public:
  typedef T cref_type;

  sequence_instruction_ex(presence_enum_t optional, uint32_t id,
                          const char *name, const char *ns,
                          const char *dictionary,
                          instructions_view_t subinstructions,
                          const group_field_instruction *element_instruction,
                          const group_field_instruction *ref_instruction,
                          uint32_field_instruction *sequence_length_instruction,
                          const char *typeref_name, const char *typeref_ns,
                          const char *cpp_ns,
                          instruction_tag tag = instruction_tag())
      : sequence_field_instruction(optional, id, name, ns, dictionary,
                                   subinstructions, element_instruction,
                                   ref_instruction, sequence_length_instruction,
                                   typeref_name, typeref_ns, cpp_ns, tag) {}

  virtual sequence_instruction_ex<T> *
  clone(arena_allocator &alloc) const override {
    return new (alloc) sequence_instruction_ex<T>(*this);
  }
};

} /* mfast */
