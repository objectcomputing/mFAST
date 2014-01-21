// Copyright (c) 2013, Huang-Ming Huang,  Object Computing, Inc.
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

#include <cstddef>
#include <cassert>
#include <boost/static_assert.hpp>
#include "value_storage.h"
#include "mfast/mfast_export.h"
#include "mfast/arena_allocator.h"
#include <algorithm>

namespace mfast {

  class allocator;

  enum operator_enum_t {
    operator_none,
    operator_constant,
    operator_delta,
    operator_default,
    operator_copy,
    operator_increment,
    operator_tail,
    operators_count
  };

  enum presence_enum_t {
    presence_mandatory =0,
    presence_optional=1
  };

  enum field_type_enum_t {
    field_type_int32,
    field_type_uint32,
    field_type_int64,
    field_type_uint64,
    field_type_decimal,
    field_type_exponent,
    field_type_templateref,
    field_type_ascii_string, // using of_array start
    field_type_unicode_string,
    field_type_byte_vector,
    field_type_int32_vector,
    field_type_uint32_vector,
    field_type_int64_vector,
    field_type_uint64_vector,
    field_type_sequence, // using of_array end, codegen needed start
    field_type_group,
    field_type_template,
    field_type_enum
  };


  struct op_context_t {
    const char* key_;
    const char* ns_;
    const char* dictionary_;
  };


  template <typename T>
  struct field_type_trait;

  template <>
  struct field_type_trait<int32_t>
  {
    enum { id = field_type_int32};

  };

  template <>
  struct field_type_trait<uint32_t>
  {
    enum { id = field_type_uint32};

  };

  template <>
  struct field_type_trait<int64_t>
  {
    enum { id = field_type_int64};

  };

  template <>
  struct field_type_trait<uint64_t>
  {
    enum { id = field_type_uint64};

  };

  class field_instruction_visitor;


  class MFAST_EXPORT field_instruction
  {
  public:
    virtual void construct_value(value_storage& storage,
                                 allocator*     alloc) const=0;
    virtual void destruct_value(value_storage& storage,
                                allocator*     alloc) const;

    /// Perform deep copy
    virtual void copy_construct_value(const value_storage& src,
                                      value_storage&       dest,
                                      allocator*           alloc,
                                      value_storage*       fields_storage=0) const;

    virtual void accept(field_instruction_visitor&, void*) const=0;
    virtual field_instruction* clone(arena_allocator& alloc) const =0;
    virtual std::size_t pmap_size() const;

    bool is_nullable() const
    {
      return nullable_flag_;
    }

    bool optional() const
    {
      return optional_flag_;
    }

    uint32_t id() const
    {
      return id_;
    }

    const char* name() const
    {
      return name_;
    }

    const char* ns() const
    {
      return ns_;
    }

    field_type_enum_t field_type() const
    {
      return static_cast<field_type_enum_t>(field_type_);
    }

    operator_enum_t field_operator() const
    {
      return static_cast<operator_enum_t>(operator_id_);
    }

    uint16_t field_index() const
    {
      return field_index_;
    }

    void field_index(uint16_t i)
    {
      field_index_ = i;
    }

    bool mandatory_without_initial_value() const
    {
      return mandatory_no_initial_value_;
    }

    /// @returns true if the field type is string, byteVector or sequence.
    bool is_array() const
    {
      return is_array_;
    }

    const char* field_type_name() const;

    field_instruction(uint16_t        field_index,
                      operator_enum_t operator_id,
                      int             field_type,
                      presence_enum_t optional,
                      uint32_t        id,
                      const char*     name,
                      const char*     ns);


    void optional(bool v)
    {
      optional_flag_ = v;
      update_invariant();
    }

    void id(uint32_t v)
    {
      id_ = v;
    }

    void name(const char* v)
    {
      name_ = v;
    }

    void ns(const char* v)
    {
      ns_ = v;
    }

    void field_operator(operator_enum_t v)
    {
      operator_id_ = v;
      update_invariant();
    }

  protected:

    virtual void update_invariant()
    {
      nullable_flag_ =  optional_flag_ &&  (operator_id_ != operator_constant);
      has_pmap_bit_ = operator_id_ > operator_delta || ((operator_id_ == operator_constant) && optional_flag_);
    }

    uint16_t field_index_;
    uint16_t operator_id_ : 3;
    uint16_t is_array_ : 1;
    uint16_t optional_flag_ : 1;
    uint16_t nullable_flag_ : 1;
    uint16_t has_pmap_bit_ : 1;
    uint16_t mandatory_no_initial_value_ : 1;
    uint16_t field_type_ : 8;
    uint32_t id_;
    const char* name_;
    const char* ns_;
  };


  class dictionary_builder;

  class MFAST_EXPORT integer_field_instruction_base
    : public field_instruction
  {
  public:
    integer_field_instruction_base(uint16_t             field_index,
                                   operator_enum_t      operator_id,
                                   int                  field_type,
                                   presence_enum_t      optional,
                                   uint32_t             id,
                                   const char*          name,
                                   const char*          ns,
                                   const op_context_t*  context,
                                   const value_storage& initial_storage)
      : field_instruction(field_index, operator_id, field_type, optional, id, name, ns)
      , op_context_(context)
      , initial_value_(initial_storage)
      , prev_value_(&prev_storage_)
      , initial_or_default_value_(initial_storage.is_empty() ? &default_value_ : &initial_value_)
    {
      mandatory_no_initial_value_ = !optional && initial_storage.is_empty();
    }

    integer_field_instruction_base(const integer_field_instruction_base& other)
      : field_instruction(other)
      , op_context_(other.op_context_)
      , initial_value_(other.initial_value_)
      , prev_value_(&prev_storage_)
      , initial_or_default_value_(initial_value_.is_empty() ? &default_value_ : &initial_value_)
    {
    }

    virtual void construct_value(value_storage& storage,
                                 allocator*     alloc) const;



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

    void initial_value(const value_storage& v)
    {
      initial_value_ = v;
      initial_or_default_value_ = initial_value_.is_empty() ? &default_value_ : &initial_value_;
      mandatory_no_initial_value_ = !optional() && initial_value_.is_empty();
    }

  protected:
    friend class dictionary_builder;
    const op_context_t* op_context_;
    value_storage initial_value_;
    value_storage* prev_value_;
    value_storage prev_storage_;
    const value_storage* initial_or_default_value_;
    static const value_storage default_value_;

    virtual void update_invariant()
    {
      field_instruction::update_invariant();
      mandatory_no_initial_value_ = !optional() && initial_value_.is_empty();
    }

  };

  template <typename T>
  class int_field_instruction
    : public integer_field_instruction_base
  {
  public:
    int_field_instruction(uint16_t             field_index,
                          operator_enum_t      operator_id,
                          presence_enum_t      optional,
                          uint32_t             id,
                          const char*          name,
                          const char*          ns,
                          const op_context_t*  context,
                          int_value_storage<T> initial_value = int_value_storage<T>())
      : integer_field_instruction_base(field_index,
                                       operator_id,
                                       field_type_trait<T>::id,
                                       optional,
                                       id,
                                       name,
                                       ns,
                                       context,
                                       initial_value.storage_)
    {
    }

    int_field_instruction(const int_field_instruction& other)
      : integer_field_instruction_base(other)
    {
    }

    virtual void accept(field_instruction_visitor& visitor, void* context) const;
    virtual int_field_instruction<T>* clone(arena_allocator& alloc) const;
  };


  typedef int_field_instruction<int32_t> int32_field_instruction;
  typedef int_field_instruction<uint32_t> uint32_field_instruction;
  typedef int_field_instruction<int64_t> int64_field_instruction;
  typedef int_field_instruction<uint64_t> uint64_field_instruction;

  class MFAST_EXPORT mantissa_field_instruction
    : public int64_field_instruction
  {
  public:

    mantissa_field_instruction(operator_enum_t            operator_id,
                               const op_context_t*        context,
                               int_value_storage<int64_t> initial_value)
      : int64_field_instruction(0, operator_id, presence_mandatory, 0, 0, 0, context, initial_value)
    {
    }

    mantissa_field_instruction(const mantissa_field_instruction& other)
      : int64_field_instruction(other)
    {
    }

  };


  class MFAST_EXPORT decimal_field_instruction
    : public integer_field_instruction_base
  {
  public:

    decimal_field_instruction(uint16_t              field_index,
                              operator_enum_t       decimal_operator_id,
                              presence_enum_t       optional,
                              uint32_t              id,
                              const char*           name,
                              const char*           ns,
                              const op_context_t*   decimal_context,
                              decimal_value_storage initial_value = decimal_value_storage())
      : integer_field_instruction_base(field_index,
                                       decimal_operator_id,
                                       field_type_decimal,
                                       optional,
                                       id,
                                       name,
                                       ns,
                                       decimal_context,
                                       initial_value.storage_)
      , mantissa_instruction_(0)
    {
    }

    decimal_field_instruction(uint16_t                    field_index,
                              operator_enum_t             exponent_operator_id,
                              presence_enum_t             optional,
                              uint32_t                    id,
                              const char*                 name,
                              const char*                 ns,
                              const op_context_t*         exponent_context,
                              mantissa_field_instruction* mi,
                              decimal_value_storage       initial_value = decimal_value_storage())
      : integer_field_instruction_base(field_index,
                                       exponent_operator_id,
                                       field_type_exponent,
                                       optional,
                                       id,
                                       name,
                                       ns,
                                       exponent_context,
                                       initial_value.storage_)
    {
      mantissa_instruction(mi);
    }

    decimal_field_instruction(const decimal_field_instruction& other,
                              mantissa_field_instruction*      mantissa_instruction)
      : integer_field_instruction_base(other)
      , mantissa_instruction_(mantissa_instruction)
    {
    }

    /// Perform deep copy
    virtual void copy_construct_value(const value_storage& src,
                                      value_storage&       dest,
                                      allocator*           alloc,
                                      value_storage*       fields_storage=0) const;


    virtual void construct_value(value_storage& storage,
                                 allocator*     alloc) const;


    virtual void accept(field_instruction_visitor&, void*) const;
     virtual decimal_field_instruction* clone(arena_allocator& alloc) const;

    const mantissa_field_instruction* mantissa_instruction() const
    {
      return mantissa_instruction_;
    }

    const value_storage& initial_or_default_value() const
    {
      if (initial_value_.is_empty()) {
        static const decimal_value_storage default_value(0,0);
        return default_value.storage_;
      }
      return initial_value_;
    }

    void mantissa_instruction(mantissa_field_instruction* v)
    {
      mantissa_instruction_ = v;
      this->initial_value_.of_decimal.mantissa_ = mantissa_instruction_->initial_value().get<int64_t>();

      if (has_pmap_bit_ == 0) {
        has_pmap_bit_ = mantissa_instruction_->pmap_size();
      }
    }

  protected:

    friend class dictionary_builder;
    mantissa_field_instruction* mantissa_instruction_;
  };


  template <typename T>
  class referalbe_instruction
  {
  public:
    referalbe_instruction(const T*    ref_instruction,
                          const char* cpp_ns)
      : ref_instruction_(ref_instruction)
      , cpp_ns_(cpp_ns)
    {
    }

    const T* ref_instruction() const
    {
      return ref_instruction_;
    }

    void ref_instruction(const T* r)
    {
      ref_instruction_ = r;
    }

    const char* cpp_ns() const
    {
      return cpp_ns_;
    }

  private:
    const T* ref_instruction_;
    const char* cpp_ns_;
  };

  class MFAST_EXPORT enum_field_instruction
    : public integer_field_instruction_base
    , public referalbe_instruction<enum_field_instruction>
  {
  public:

    enum_field_instruction(uint16_t                      field_index,
                           operator_enum_t               operator_id,
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
                           const char*                   cpp_ns)
      : integer_field_instruction_base(field_index,
                                       operator_id,
                                       field_type_enum,
                                       optional,
                                       id,
                                       name,
                                       ns,
                                       context,
                                       initial_value.storage_)
      , referalbe_instruction<enum_field_instruction>(ref, cpp_ns)
      , elements_(elements)
      , num_elements_(num_elements)
      , element_values_(element_values)
    {
    }

    enum_field_instruction(const enum_field_instruction &other)
      : integer_field_instruction_base(other)
      , referalbe_instruction<enum_field_instruction>(other)
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

  class MFAST_EXPORT vector_field_instruction_base
    : public field_instruction
  {
  public:

    vector_field_instruction_base(uint16_t          field_index,
                                  operator_enum_t   operator_id,
                                  field_type_enum_t field_type,
                                  presence_enum_t   optional,
                                  uint32_t          id,
                                  const char*       name,
                                  const char*       ns,
                                  std::size_t       element_size)
      : field_instruction(field_index,
                          operator_id,
                          field_type,
                          optional,
                          id,
                          name,
                          ns)
      , element_size_(element_size)
    {
    }

    vector_field_instruction_base(const vector_field_instruction_base& other)
      : field_instruction(other)
      , element_size_(other.element_size_)
    {
    }

    virtual void construct_value(value_storage& storage,
                                 allocator*     alloc) const;
    virtual void destruct_value(value_storage& storage,
                                allocator*     alloc) const;


    // perform deep copy
    virtual void copy_construct_value(const value_storage& src,
                                      value_storage&       dest,
                                      allocator*           alloc,
                                      value_storage*       fields_storage=0) const;

  protected:
    std::size_t element_size_;
  };


  class MFAST_EXPORT ascii_field_instruction
    : public vector_field_instruction_base
  {
  public:
    ascii_field_instruction(uint16_t             field_index,
                            operator_enum_t      operator_id,
                            presence_enum_t      optional,
                            uint32_t             id,
                            const char*          name,
                            const char*          ns,
                            const op_context_t*  context,
                            string_value_storage initial_value = string_value_storage(),
                            field_type_enum_t    field_type = field_type_ascii_string)
      :  vector_field_instruction_base(field_index,
                                       operator_id,
                                       field_type,
                                       optional,
                                       id, name, ns,
                                       sizeof(char))
      , op_context_(context)
      , initial_value_(initial_value.storage_)
      , prev_value_(&prev_storage_)
      , initial_or_default_value_(initial_value_.is_empty() ? &default_value_ : &initial_value_)
    {
      mandatory_no_initial_value_ = !optional && initial_value.storage_.is_empty();
    }

    ascii_field_instruction(const ascii_field_instruction& other)
      : vector_field_instruction_base(other)
      , op_context_(other.op_context_)
      , initial_value_(other.initial_value_)
      , prev_value_(&prev_storage_)
      , initial_or_default_value_(initial_value_.is_empty() ? &default_value_ : &initial_value_)
    {
    }

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

    void initial_value(const value_storage& v)
    {
      initial_value_ = v;
      initial_or_default_value_ = initial_value_.is_empty() ? &default_value_ : &initial_value_;
      mandatory_no_initial_value_ = !optional() && initial_value_.is_empty();
    }

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
    unicode_field_instruction(uint16_t             field_index,
                              operator_enum_t      operator_id,
                              presence_enum_t      optional,
                              uint32_t             id,
                              const char*          name,
                              const char*          ns,
                              const op_context_t*  context,
                              string_value_storage initial_value = string_value_storage(),
                              uint32_t             length_id = 0,
                              const char*          length_name = "",
                              const char*          length_ns = "",
                              field_type_enum_t    field_type = field_type_unicode_string)
      :  ascii_field_instruction(field_index,
                                 operator_id,
                                 optional,
                                 id, name, ns, context,
                                 initial_value,
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

  protected:
    uint32_t length_id_;
    const char* length_name_;
    const char* length_ns_;
  };


  class MFAST_EXPORT byte_vector_field_instruction
    : public unicode_field_instruction
  {
  public:
    byte_vector_field_instruction(uint16_t             field_index,
                                  operator_enum_t      operator_id,
                                  presence_enum_t      optional,
                                  uint32_t             id,
                                  const char*          name,
                                  const char*          ns,
                                  const op_context_t*  context,
                                  string_value_storage initial_value = string_value_storage(),
                                  uint32_t             length_id = 0,
                                  const char*          length_name = "",
                                  const char*          length_ns = "")
      :  unicode_field_instruction(field_index,
                                   operator_id,
                                   optional,
                                   id, name, ns, context,
                                   initial_value,
                                   length_id,
                                   length_name,
                                   length_ns,
                                   field_type_byte_vector)
    {
    }

    byte_vector_field_instruction(const byte_vector_field_instruction& other)
      : unicode_field_instruction(other)
    {
    }

    virtual void accept(field_instruction_visitor& visitor, void* context) const;
    virtual byte_vector_field_instruction* clone(arena_allocator& alloc) const;
  };


  namespace detail {
    template <typename T>
    struct vector_field_type;


    template <>
    struct vector_field_type<int32_t>
    {
      static const field_type_enum_t value = field_type_int32_vector;
    };

    template <>
    struct vector_field_type<uint32_t>
    {
      static const field_type_enum_t value = field_type_uint32_vector;
    };

    template <>
    struct vector_field_type<int64_t>
    {
      static const field_type_enum_t value = field_type_int64_vector;
    };

    template <>
    struct vector_field_type<uint64_t>
    {
      static const field_type_enum_t value = field_type_uint64_vector;
    };

  }

  template <typename T>
  class vector_field_instruction
    : public vector_field_instruction_base
  {
  public:
    vector_field_instruction(uint16_t        field_index,
                             presence_enum_t optional,
                             uint32_t        id,
                             const char*     name,
                             const char*     ns)
      :  vector_field_instruction_base(field_index,
                                       operator_none,
                                       detail::vector_field_type<T>::value,
                                       optional,
                                       id, name, ns, sizeof(T))

    {
    }

    vector_field_instruction(const vector_field_instruction& other)
      :  vector_field_instruction_base(other)
    {
    }

    virtual void accept(field_instruction_visitor& visitor, void* context) const;
    virtual vector_field_instruction<T>* clone(arena_allocator& alloc) const;
  };

  typedef vector_field_instruction<int32_t> int32_vector_field_instruction;
  typedef vector_field_instruction<uint32_t> uint32_vector_field_instruction;
  typedef vector_field_instruction<int64_t> int64_vector_field_instruction;
  typedef vector_field_instruction<uint64_t> uint64_vector_field_instruction;

  typedef const field_instruction*  const_instruction_ptr_t;

  class MFAST_EXPORT aggregate_instruction_base
    : public field_instruction
  {
  public:
    aggregate_instruction_base(uint16_t                       field_index,
                               presence_enum_t                optional,
                               uint32_t                       id,
                               const char*                    name,
                               const char*                    ns,
                               const char*                    dictionary,
                               const const_instruction_ptr_t* subinstructions,
                               uint32_t                       subinstructions_count,
                               const char*                    typeref_name ="",
                               const char*                    typeref_ns="")
      : field_instruction(field_index,
                          operator_none,
                          field_type_group,
                          optional,
                          id,
                          name,
                          ns)
      , dictionary_(dictionary)
      , typeref_name_(typeref_name)
      , typeref_ns_(typeref_ns)
      , segment_pmap_size_(0)
    {
      set_subinstructions(subinstructions, subinstructions_count);
    }

    void construct_group_subfields(value_storage* group_content,
                                   allocator*     alloc) const;
    void destruct_group_subfields(value_storage* group_content,
                                  allocator*     alloc) const;

    void copy_group_subfields(const value_storage* src,
                              value_storage*       dest,
                              allocator*           alloc) const;

    /// Returns the number of bytes needed for the content of the group
    uint32_t group_content_byte_count() const
    {
      return subinstructions_count_ * sizeof(value_storage);
    }

    /// Returns the index for the subinstruction with specified id,
    /// or -1 if not found.
    int find_subinstruction_index_by_id(uint32_t id) const;

    /// Returns the index for the subinstruction with specified name,
    /// or -1 if not found.
    int find_subinstruction_index_by_name(const char* name) const;

    uint32_t subinstructions_count() const
    {
      return subinstructions_count_;
    }

    const field_instruction* subinstruction(std::size_t index) const
    {
      assert(index < subinstructions_count_);
      return subinstructions_[index];
    }

    std::size_t segment_pmap_size() const
    {
      return segment_pmap_size_;
    }

    void set_subinstructions(const const_instruction_ptr_t* subinstructions, uint32_t count)
    {
      subinstructions_ = subinstructions;
      subinstructions_count_ = count;
      segment_pmap_size_ = 0;
      for (uint32_t i = 0; i < subinstructions_count_; ++i) {
        segment_pmap_size_ += subinstruction(i)->pmap_size();
      }
      has_pmap_bit_ = segment_pmap_size() > 0 ? 1 : 0;
    }

    const const_instruction_ptr_t*  subinstructions() const
    {
      return subinstructions_;
    }

    const char* typeref_name() const
    {
      return typeref_name_;
    }

    void typeref_name(const char* v)
    {
      typeref_name_ = v;
    }

    const char* typeref_ns() const
    {
      return typeref_ns_;
    }

    void typeref_ns(const char* v)
    {
      typeref_ns_ = v;
    }

    const char* dictionary() const
    {
      return dictionary_;
    }

    void dictionary(const char* v)
    {
      dictionary_ = v;
    }

  protected:

    const char* dictionary_;
    uint32_t subinstructions_count_;
    const char* typeref_name_;
    const char* typeref_ns_;
    std::size_t segment_pmap_size_;
    const const_instruction_ptr_t* subinstructions_;
  };


  class template_instruction;

  class MFAST_EXPORT group_field_instruction
    : public aggregate_instruction_base
    , public referalbe_instruction<group_field_instruction>
  {
  public:

    group_field_instruction(uint16_t                       field_index,
                            presence_enum_t                optional,
                            uint32_t                       id,
                            const char*                    name,
                            const char*                    ns,
                            const char*                    dictionary,
                            const const_instruction_ptr_t* subinstructions,
                            uint32_t                       subinstructions_count,
                            const char*                    typeref_name ="",
                            const char*                    typeref_ns="",
                            const char*                    cpp_ns="")
      : aggregate_instruction_base(field_index,
                                   optional,
                                   id,
                                   name,
                                   ns,
                                   dictionary,
                                   subinstructions,
                                   subinstructions_count,
                                   typeref_name,
                                   typeref_ns)
      , referalbe_instruction<group_field_instruction>(0, cpp_ns)
    {
    }

    virtual void construct_value(value_storage& storage,
                                 allocator*     alloc) const;
    virtual void destruct_value(value_storage& storage,
                                allocator*     alloc) const;

// #ifdef __clang__
// #pragma clang diagnostic push
// #pragma clang diagnostic ignored "-Woverloaded-virtual"
// #endif

    virtual void construct_value(value_storage& storage,
                                 value_storage* fields_storage,
                                 allocator*     alloc,
                                 bool           construct_subfields=true) const;
// #ifdef __clang__
// #pragma clang diagnostic pop
// #endif

    // perform deep copy
    virtual void copy_construct_value(const value_storage& src,
                                      value_storage&       dest,
                                      allocator*           alloc,
                                      value_storage*       fields_storage=0) const;

    virtual void accept(field_instruction_visitor&, void*) const;
    virtual group_field_instruction* clone(arena_allocator& alloc) const;


  };


  class MFAST_EXPORT sequence_field_instruction
    : public group_field_instruction
  {
  public:
    sequence_field_instruction(uint16_t                        field_index,
                               presence_enum_t                 optional,
                               uint32_t                        id,
                               const char*                     name,
                               const char*                     ns,
                               const char*                     dictionary,
                               const const_instruction_ptr_t*  subinstructions,
                               uint32_t                        subinstructions_count,
                               const uint32_field_instruction* sequence_length_instruction,
                               const char*                     typeref_name="",
                               const char*                     typeref_ns="",
                               const char*                     cpp_ns="")
      : group_field_instruction(field_index,
                                optional,
                                id,
                                name,
                                ns,
                                dictionary,
                                subinstructions,
                                subinstructions_count,
                                typeref_name,
                                typeref_ns,
                                cpp_ns)
      , sequence_length_instruction_(sequence_length_instruction)
    {
      field_type_ = field_type_sequence;
    }

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

  private:

    friend class dictionary_builder;
    const uint32_field_instruction* sequence_length_instruction_;
  };


  class MFAST_EXPORT template_instruction
    : public group_field_instruction
  {
  public:
    template_instruction(uint32_t                       id,
                         const char*                    name,
                         const char*                    ns,
                         const char*                    template_ns,
                         const char*                    dictionary,
                         const const_instruction_ptr_t* subinstructions,
                         uint32_t                       subinstructions_count,
                         bool                           reset,
                         const char*                    typeref_name="",
                         const char*                    typeref_ns="",
                         const char*                    cpp_ns="")
      : group_field_instruction(0, presence_mandatory,
                                id,
                                name,
                                ns,
                                dictionary,
                                subinstructions,
                                subinstructions_count,
                                typeref_name,
                                typeref_ns,
                                cpp_ns)
      , template_ns_(template_ns)
      , reset_(reset)
    {
      field_type_ = field_type_template;
    }

    const char* template_ns() const
    {
      return template_ns_;
    }

    void copy_construct_value(value_storage&       storage,
                              value_storage*       fields_storage,
                              allocator*           alloc,
                              const value_storage* src_fields_storage) const;

    virtual void copy_construct_value(const value_storage& src,
                                      value_storage&       dest,
                                      allocator*           alloc,
                                      value_storage*       fields_storage=0) const;


    virtual void accept(field_instruction_visitor&, void*) const;
    virtual template_instruction* clone(arena_allocator& alloc) const;

    bool has_reset_attribute() const
    {
      return reset_;
    }

    // void ensure_valid_storage(value_storage& storage,
    //                           allocator*     alloc) const;

  private:
    const char* template_ns_;
    bool reset_;
  };


  template <typename T>
  class group_instruction_ex
    : public group_field_instruction
  {
  public:
    group_instruction_ex(uint16_t                       field_index,
                         presence_enum_t                optional,
                         uint32_t                       id,
                         const char*                    name,
                         const char*                    ns,
                         const char*                    dictionary,
                         const const_instruction_ptr_t* subinstructions,
                         uint32_t                       subinstructions_count,
                         const char*                    typeref_name ="",
                         const char*                    typeref_ns="")
      : group_field_instruction(field_index,
                                optional,
                                id,
                                name,
                                ns,
                                dictionary,
                                subinstructions,
                                subinstructions_count,
                                typeref_name,
                                typeref_ns)
    {
    }

    group_instruction_ex(uint16_t                    field_index,
                         presence_enum_t             optional,
                         uint32_t                    id,
                         const char*                 name,
                         const char*                 ns,
                         const char*                 dictionary,
                         const template_instruction* ref_instruction,
                         const char*                 typeref_name ="",
                         const char*                 typeref_ns="")
      : group_field_instruction(field_index,
                                optional,
                                id,
                                name,
                                ns,
                                dictionary,
                                ref_instruction->subinstructions(),
                                ref_instruction->subinstructions_count(),
                                typeref_name,
                                typeref_ns)
    {
    }

    virtual group_instruction_ex<T>* clone(arena_allocator& alloc) const
    {
      return new (alloc) group_instruction_ex<T>(*this);
    }

  };


  template <typename T>
  class sequence_instruction_ex
    : public sequence_field_instruction
  {
  public:
    sequence_instruction_ex(uint16_t                       field_index,
                            presence_enum_t                optional,
                            uint32_t                       id,
                            const char*                    name,
                            const char*                    ns,
                            const char*                    dictionary,
                            const const_instruction_ptr_t* subinstructions,
                            uint32_t                       subinstructions_count,
                            uint32_field_instruction*      sequence_length_instruction,
                            const char*                    typeref_name="",
                            const char*                    typeref_ns="")
      : sequence_field_instruction(field_index, optional, id, name, ns, dictionary, subinstructions,
                                   subinstructions_count, sequence_length_instruction, typeref_name, typeref_ns)
    {
    }

    sequence_instruction_ex(uint16_t                    field_index,
                            presence_enum_t             optional,
                            uint32_t                    id,
                            const char*                 name,
                            const char*                 ns,
                            const char*                 dictionary,
                            const template_instruction* ref_instruction,
                            uint32_field_instruction*   sequence_length_instruction,
                            const char*                 typeref_name="",
                            const char*                 typeref_ns="")
      : sequence_field_instruction(field_index, optional, id, name, ns, dictionary,
                                   ref_instruction->subinstructions(),
                                   ref_instruction->subinstructions_count(),
                                   sequence_length_instruction, typeref_name, typeref_ns)
    {
    }

    virtual sequence_instruction_ex<T>* clone(arena_allocator& alloc) const
    {
      return new (alloc) sequence_instruction_ex<T>(*this);
    }
  };


  template <typename T>
  class template_instruction_ex
    : public template_instruction
  {
  public:
    template_instruction_ex(uint32_t                       id,
                            const char*                    name,
                            const char*                    ns,
                            const char*                    template_ns,
                            const char*                    dictionary,
                            const const_instruction_ptr_t* subinstructions,
                            uint32_t                       subinstructions_count,
                            bool                           reset,
                            const char*                    typeref_name="",
                            const char*                    typeref_ns="")
      : template_instruction(id, name, ns, template_ns, dictionary,
                             subinstructions, subinstructions_count, reset, typeref_name, typeref_ns)
    {
    }

    virtual template_instruction_ex<T>* clone(arena_allocator& alloc) const
    {
      return new (alloc) template_instruction_ex<T>(*this);
    }

  };

  class MFAST_EXPORT templateref_instruction
    : public field_instruction
  {
  public:


    templateref_instruction(uint16_t                    field_index,
                            const template_instruction* ref =0)
      : field_instruction(field_index, operator_none,
                          field_type_templateref,
                          presence_mandatory,
                          0,
                          ref ? ref->name() : "",
                          ref ? ref->ns() : "")
      , target_(ref)
    {
    }

    virtual void construct_value(value_storage& storage,
                                 allocator*     alloc) const;
    virtual void destruct_value(value_storage& storage,
                                allocator*     alloc) const;

    void  construct_value(value_storage&              storage,
                          allocator*                  alloc,
                          const template_instruction* from_inst,
                          bool                        construct_subfields) const;

    virtual std::size_t pmap_size() const;

    /// Perform deep copy
    virtual void copy_construct_value(const value_storage& src,
                                      value_storage&       dest,
                                      allocator*           alloc,
                                      value_storage*       fields_storage=0) const;

    virtual void accept(field_instruction_visitor&, void*) const;

    const template_instruction* target() const
    {
      return target_;
    }
    virtual templateref_instruction* clone(arena_allocator& alloc) const;

    static const const_instruction_ptr_t* default_instruction();

  private:
    const template_instruction* target_;
  };

  namespace coder {
    class templates_builder;
  }
  class MFAST_EXPORT templates_description
  {
  public:
    template <unsigned SIZE>
    templates_description(
      const char* ns,
      const char* template_ns,
      const char* dictionary,
      const template_instruction* (&instructions)[SIZE])
      : ns_(ns)
      , template_ns_(template_ns)
      , dictionary_(dictionary)
      , instructions_(instructions)
      , instructions_count_(SIZE)
    {
    }

    typedef const template_instruction** iterator;

    const char* ns() const
    {
      return ns_;
    }

    const char* template_ns() const
    {
      return template_ns_;
    }

    const char* dictionary() const
    {
      return dictionary_;
    }

    const template_instruction* operator[](std::size_t i) const
    {
      return instructions_[i];
    }

    iterator begin() const
    {
      return instructions_;
    }

    iterator end() const
    {
      return instructions_+ size();
    }

    const template_instruction* instruction_with_id(uint32_t id) const
    {
      for (uint32_t i = 0; i <  instructions_count_; ++i) {
        if (instructions_[i]->id() == id)
          return instructions_[i];
      }
      return 0;
    }

    uint32_t size() const
    {
      return instructions_count_;
    }

  protected:
    templates_description()
    {
    }

    friend class coder::templates_builder;
    const char* ns_;
    const char* template_ns_;
    const char* dictionary_;
    const template_instruction** instructions_;
    uint32_t instructions_count_;
  };


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
  int_field_instruction<T>*
  int_field_instruction<T>::clone(arena_allocator& alloc) const
  {
    return new (alloc) int_field_instruction<T>(*this);
  }


  template <typename T>
  void vector_field_instruction<T>::accept(field_instruction_visitor& visitor, void* context) const
  {
    visitor.visit(this, context);
  }

  template <typename T>
  vector_field_instruction<T>*
  vector_field_instruction<T>::clone(arena_allocator& alloc) const
  {
    return new (alloc) vector_field_instruction<T>(*this);
  }


  template <typename T>
  struct instruction_trait;


  template <>
  struct instruction_trait<int32_t>
  {
    typedef int32_field_instruction type;
  };

  template <>
  struct instruction_trait<uint32_t>
  {
    typedef uint32_field_instruction type;
  };

  template <>
  struct instruction_trait<int64_t>
  {
    typedef int64_field_instruction type;
  };

  template <>
  struct instruction_trait<uint64_t>
  {
    typedef uint64_field_instruction type;
  };

///////////////////////////////////////////////////////
} // namespace mfast
#endif /* end of include guard: FIELD_INSTRUCTION_H_PMKVDZOC */
