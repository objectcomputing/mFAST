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

#ifndef ENUM_REF_H_USHYGHOA
#define ENUM_REF_H_USHYGHOA

#include "mfast/field_instructions.h"
#include "mfast/field_ref.h"

namespace mfast {

namespace detail {
class codec_helper;
}

class MFAST_EXPORT enum_cref
  : public field_cref
{
public:
  typedef uint64_t value_type;
  typedef enum_field_instruction instruction_type;
  typedef const instruction_type* instruction_cptr;

  enum_cref()
    : field_cref()
  {
  }

  enum_cref(const value_storage* storage,
            instruction_cptr     instruction)
    : field_cref(storage, instruction)
  {
  }

  enum_cref(const enum_cref& other)
    : field_cref(other)
  {
  }

  explicit enum_cref(const field_cref& other)
    : field_cref(other)
  {
  }

  uint32_t id() const
  {
    return instruction_->id();
  }

  bool is_initial_value() const
  {
    return (this->absent() == this->instruction()->initial_value().is_empty() &&
            (this->absent() || value() == this->instruction()->initial_value().get<uint64_t>()));
  }

  uint64_t value() const
  {
    return storage_->get<uint64_t>();
  }

  const char* value_name() const
  {
    // assert(instruction() && value() < instruction()->num_elements_);
    return instruction()->element_name(value());
  }

  instruction_cptr instruction() const
  {
    return static_cast<instruction_cptr>(instruction_);
  }

  bool is_boolean() const;
private:

  enum_cref& operator = (const enum_cref&);
};

inline bool operator == (const enum_cref& lhs, const enum_cref& rhs)
{
  return (lhs.absent() == rhs.absent()) &&
         (lhs.absent() || lhs.value() == rhs.value());
}

inline bool operator != (const enum_cref& lhs, const enum_cref& rhs)
{
  return !(lhs == rhs);
}

class fast_istream;

class enum_mref
  : public make_field_mref<enum_cref>
{
  typedef make_field_mref<enum_cref> base_type;

public:

  enum_mref()
  {
  }

  enum_mref(mfast::allocator* alloc,
            value_storage*    storage,
            instruction_cptr  instruction)
    : base_type(alloc, storage, instruction)
  {
  }

  enum_mref(const enum_mref& other)
    : base_type(other)
  {
  }

  void as (const enum_cref& cref) const
  {
    if (cref.absent()) {
      this->omit();
    }
    else {
      as(cref.value());
    }
  }

  void as(uint64_t v) const
  {
    this->storage()->present(1);
    this->storage()->set(v);
  }

  void to_initial_value() const
  {
    *this->storage() = this->instruction()->initial_value();
  }

  uint64_t value() const
  {
    return this->storage()->get<uint64_t>();
  }

};


template <>
struct mref_of<enum_cref>
{
  typedef enum_mref type;
};


template <typename Tag>
class enum_field_instruction_ex
  : public enum_field_instruction
{
public:
  enum_field_instruction_ex(operator_enum_t               operator_id,
                            presence_enum_t               optional,
                            uint32_t                      id,
                            const char*                   name,
                            const char*                   ns,
                            const op_context_t*           context,
                            int_value_storage<uint64_t>   initial_value,
                            const char**                  element_names,
                            const uint64_t*               element_values,
                            uint64_t                      num_elements,
                            const enum_field_instruction* ref,
                            const char*                   cpp_ns,
                            instruction_tag               tag=instruction_tag())
    : enum_field_instruction(operator_id,
                             optional,
                             id,
                             name,
                             ns,
                             context,
                             initial_value,
                             element_names,
                             element_values,
                             num_elements,
                             ref,
                             cpp_ns,
                             tag)
  {
  }

};

template <typename Derived, typename EnumClassType>
class enum_cref_ex
  : public enum_cref
{
public:
  typedef typename EnumClassType::element element_type;
  typedef typename EnumClassType::instruction_type instruction_type;
  typedef const instruction_type* intruction_cptr;

  enum_cref_ex(const value_storage*          storage,
               const enum_field_instruction* instruction)
    : enum_cref(storage, instruction)
  {
  }

  enum_cref_ex(const field_cref& other)
    : enum_cref(other)
  {
  }

  bool operator == (const Derived& v) const
  {
    return this->value() == v.value();
  }

  bool operator == (element_type v) const
  {
    return this->value() == v;
  }

  bool operator != (const Derived& v) const
  {
    return !( *this == v);
  }

  bool operator != (element_type v) const
  {
    return !( *this == v);
  }

  element_type value() const
  {
    return static_cast<element_type>(enum_cref::value());
  }

  intruction_cptr instruction() const
  {
    return static_cast<intruction_cptr>(instruction_);
  }

};

template <typename Derived, typename CRefType>
class enum_mref_ex
  : public make_field_mref<CRefType>
{
  typedef make_field_mref<CRefType> base_type;

public:

  typedef CRefType cref_type;
  typedef typename CRefType::element_type element_type;

  enum_mref_ex(mfast::allocator*             alloc,
               value_storage*                storage,
               const enum_field_instruction* instruction)
    : base_type(alloc, storage, instruction)
  {
  }

  enum_mref_ex(const field_mref_base& other)
    : base_type(other)
  {
  }

  operator enum_mref() const
  {
    return reinterpret_cast<const enum_mref&>(*this);
  }

  void as (const cref_type& ref) const
  {
    if (ref.absent()) {
      this->omit();
    }
    else {
      as(ref.value());
    }
  }

  void as(element_type v) const
  {
    this->storage()->present(1);
    this->storage()->template set<uint64_t>(v);
  }

  void to_initial_value() const
  {
    *this->storage() = this->instruction()->initial_value();
  }

};

}


#endif /* end of include guard: ENUM_REF_H_USHYGHOA */
