// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.

#pragma once

#include "vector_instruction_base.h"

namespace mfast {
namespace detail {
template <typename T> struct vector_field_type;

template <> struct vector_field_type<int32_t> {
  static const field_type_enum_t value = field_type_int32_vector;
};

template <> struct vector_field_type<uint32_t> {
  static const field_type_enum_t value = field_type_uint32_vector;
};

template <> struct vector_field_type<int64_t> {
  static const field_type_enum_t value = field_type_int64_vector;
};

template <> struct vector_field_type<uint64_t> {
  static const field_type_enum_t value = field_type_uint64_vector;
};
}

template <typename T>
class vector_field_instruction : public vector_field_instruction_base {
public:
  vector_field_instruction(presence_enum_t optional, uint32_t id,
                           const char *name, const char *ns,
                           instruction_tag tag = instruction_tag())
      : vector_field_instruction_base(operator_none,
                                      detail::vector_field_type<T>::value,
                                      optional, id, name, ns, sizeof(T), tag)

  {}

  vector_field_instruction(const vector_field_instruction &other)
      : vector_field_instruction_base(other) {}

  virtual void accept(field_instruction_visitor &visitor,
                      void *context) const override;
  virtual vector_field_instruction<T> *
  clone(arena_allocator &alloc) const override;
  static const vector_field_instruction<T> *default_instruction();
};

typedef vector_field_instruction<int32_t> int32_vector_field_instruction;
typedef vector_field_instruction<uint32_t> uint32_vector_field_instruction;
typedef vector_field_instruction<int64_t> int64_vector_field_instruction;
typedef vector_field_instruction<uint64_t> uint64_vector_field_instruction;

template <typename T>
vector_field_instruction<T> *
vector_field_instruction<T>::clone(arena_allocator &alloc) const {
  return new (alloc) vector_field_instruction<T>(*this);
}

template <typename T>
const vector_field_instruction<T> *
vector_field_instruction<T>::default_instruction() {
  static const vector_field_instruction<T> inst(presence_mandatory, 0, "", "");
  return &inst;
}

} /* mfast */
