// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.
#pragma once
#include "mfast_export.h"
#include "enum_ref.h"

namespace mfast {
struct MFAST_EXPORT boolean {
  typedef enum_field_instruction_ex<boolean> instruction_type;
  static const instruction_type *instruction();
};

class boolean_cref : public enum_cref {
public:
  typedef enum_cref base_type;
  typedef bool element_type;
  typedef boolean::instruction_type instruction_type;
  typedef const instruction_type *instruction_cptr;

  boolean_cref(const mfast::value_storage *storage = nullptr,
               instruction_cptr instruction = nullptr)
      : base_type(storage, instruction) {}

  explicit boolean_cref(const field_cref &other) : base_type(other) {}
  bool is_false() const { return !value(); }
  bool is_true() const { return value(); }
  bool value() const { return base_type::value() != 0; }
};

class boolean_mref : public make_field_mref<boolean_cref> {
public:
  typedef make_field_mref<boolean_cref> base_type;
  typedef bool element_type;
  typedef boolean::instruction_type instruction_type;
  typedef const instruction_type *instruction_cptr;

  boolean_mref(mfast::allocator *alloc = nullptr,
               mfast::value_storage *storage = nullptr,
               instruction_cptr instruction = nullptr)
      : base_type(alloc, storage, instruction) {}

  explicit boolean_mref(const mfast::field_mref_base &other)
      : base_type(other) {}
  void as_false() const { this->as(false); }
  void as_true() const { this->as(true); }
  void as(bool v) const {
    this->storage()->present(1);
    this->storage()->set<uint_fast32_t>(v);
  }
};

} /* mfast */
