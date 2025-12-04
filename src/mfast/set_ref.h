// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.
#pragma once

#include "mfast/field_ref.h"
#include "mfast/type_category.h"

namespace mfast
{
namespace detail {
  class codec_helper;
}

  class set_cref : public field_cref
  {
  public:
    using value_type = uint_fast32_t;
    using instruction_type = set_field_instruction;
    using instruction_cptr = const instruction_type*;
    using type_category = integer_type_tag;

    set_cref() = default;
    set_cref(const value_storage *storage, instruction_cptr instruction) :
      field_cref(storage, instruction) {}
    set_cref(const set_cref &other) = default;
    set_cref& operator=(const set_cref&) = delete;
    explicit set_cref(const field_cref &other) : field_cref(other) {}

    uint32_t id() const { return instruction_->id(); }
    bool is_initial_value() const
    {
      const auto init_val = this->instruction()->initial_value();
      return
        this->absent() == init_val.is_empty() &&
        (this->absent() || value() == init_val.get<uint64_t>());
    }

    value_type value() const { return storage_->get<value_type>(); }

    instruction_cptr instruction() const
    {
      return static_cast<instruction_cptr>(instruction_);
    }

  protected:
    friend class mfast::detail::codec_helper;
    void save_to(value_storage &v) const
    {
      v.of_uint64.content_ = this->storage()->of_uint64.content_;
      v.defined(true);
      v.present(this->present());
    }
  };


  inline bool operator==(const set_cref &lhs, const set_cref &rhs)
  {
    return (lhs.absent() == rhs.absent()) &&
      (lhs.absent() || lhs.value() == rhs.value());
  }

  inline bool operator!=(const set_cref &lhs, const set_cref &rhs)
  {
    return !(lhs == rhs);
  }
  class fast_istream;


  class set_mref : public make_field_mref<set_cref>
  {
    using base_type = make_field_mref<set_cref>;

  public:
    set_mref() = default;
    set_mref(mfast::allocator *alloc, value_storage *storage,
             instruction_cptr instruction) :
      base_type(alloc, storage, instruction) {}
    set_mref(const set_mref&) = default;
    set_mref& operator=(const set_mref&) = delete;
    explicit set_mref(const field_mref_base &other) : base_type(other) {}

    void as(const set_cref &cref) const
    {
      if (cref.absent())
        this->omit();
      else
        as(cref.value());
    }

    void as(value_type v) const
    {
      this->storage()->present(1);
      this->storage()->set<value_type>(v);
    }

    void to_initial_value() const
    {
      *this->storage() = this->instruction()->initial_value();
    }
    value_type value() const { return this->storage()->get<value_type>(); }

  protected:
    friend class mfast::detail::codec_helper;
    void copy_from(value_storage v) const { *this->storage() = v; }
  };

  template <> struct mref_of<set_cref> { using type = set_mref; };


  template <typename Tag>
  class set_field_instruction_ex : public set_field_instruction
  {
  public:
    set_field_instruction_ex(
      operator_enum_t operator_id, presence_enum_t optional, uint32_t id,
      const char *name, const char *ns, const op_context_t *context,
      int_value_storage<uint64_t> initial_value, const char **element_names,
      uint64_t num_elements, const set_field_instruction *ref,
      const char *cpp_ns, instruction_tag tag = instruction_tag()) :
      set_field_instruction(operator_id, optional, id, name, ns, context,
                            initial_value, element_names, num_elements, ref,
                            cpp_ns, tag) {}
  };


  template <typename Derived, typename SetClassType>
  class set_cref_ex : public set_cref
  {
  public:
    using element_type = typename SetClassType::element;
    using instruction_type = typename SetClassType::instruction_type;
    using intruction_cptr = const instruction_type*;

    set_cref_ex(const value_storage *storage,
                const set_field_instruction *instruction) :
      set_cref(storage, instruction) {}

    set_cref_ex(const field_cref &other) : set_cref(other) {}
    bool operator==(const Derived &v) const { return this->value() == v.value(); }
    bool operator==(element_type v) const { return this->value() == v; }
    bool operator!=(const Derived &v) const { return !(*this == v); }
    bool operator!=(element_type v) const { return !(*this == v); }
    element_type value() const
    {
      return static_cast<element_type>(set_cref::value());
    }
    intruction_cptr instruction() const
    {
      return static_cast<intruction_cptr>(instruction_);
    }
  };


  template <typename Derived, typename CRefType>
  class set_mref_ex : public make_field_mref<CRefType>
  {
    using base_type = make_field_mref<CRefType>;

  public:
    using cref_type = CRefType;
    using element_type = typename CRefType::element_type;

    set_mref_ex(mfast::allocator *alloc, value_storage *storage,
                const set_field_instruction *instruction) :
      base_type(alloc, storage, instruction) {}

    set_mref_ex(const field_mref_base &other) : base_type(other) {}

    operator set_mref() const
    {
      return reinterpret_cast<const set_mref&>(*this);
    }

    void as(const cref_type& ref) const
    {
      if (ref.absent())
        this->omit();
      else
        as(ref.value());
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
