// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.

#pragma once

#include "group_instruction.h"

namespace mfast {
class MFAST_EXPORT template_instruction : public group_field_instruction {
public:
  template_instruction(uint32_t id, const char *name, const char *ns,
                       const char *template_ns, const char *dictionary,
                       instructions_view_t subinstructions, bool reset,
                       const char *typeref_name, const char *typeref_ns,
                       const char *cpp_ns,
                       instruction_tag tag = instruction_tag())
      : group_field_instruction(presence_mandatory, id, name, ns, dictionary,
                                subinstructions, typeref_name, typeref_ns,
                                cpp_ns, tag),
        template_ns_(template_ns), reset_(reset) {
    field_type_ = field_type_template;
  }

  const char *template_ns() const { return template_ns_; }
  void copy_construct_value(value_storage &storage,
                            value_storage *fields_storage, allocator *alloc,
                            const value_storage *src_fields_storage) const;

  virtual void
  copy_construct_value(const value_storage &src, value_storage &dest,
                       allocator *alloc,
                       value_storage *fields_storage = nullptr) const override;

  virtual void accept(field_instruction_visitor &, void *) const override;
  virtual template_instruction *clone(arena_allocator &alloc) const override;

  bool has_reset_attribute() const { return reset_; }
  // void ensure_valid_storage(value_storage& storage,
  //                           allocator*     alloc) const;

private:
  const char *template_ns_;
  bool reset_;
};

template <typename T>
class template_instruction_ex : public template_instruction {
public:
  typedef T cref_type;
  template_instruction_ex(uint32_t id, const char *name, const char *ns,
                          const char *template_ns, const char *dictionary,
                          instructions_view_t subinstructions, bool reset,
                          const char *typeref_name, const char *typeref_ns,
                          const char *cpp_ns,
                          instruction_tag tag = instruction_tag())
      : template_instruction(id, name, ns, template_ns, dictionary,
                             subinstructions, reset, typeref_name, typeref_ns,
                             cpp_ns, tag) {}

  virtual template_instruction_ex<T> *
  clone(arena_allocator &alloc) const override {
    return new (alloc) template_instruction_ex<T>(*this);
  }
};

} /* mfast */
