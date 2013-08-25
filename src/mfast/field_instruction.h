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
  field_type_ascii_string,
  field_type_unicode_string,
  field_type_byte_vector,
  field_type_group,
  field_type_sequence,
  field_type_templateref,
  field_type_template
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
    virtual void copy_value(const value_storage& src,
                            value_storage&       dest,
                            allocator*           alloc) const;

    virtual void accept(field_instruction_visitor&, void*) const=0;
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

    bool mandatory_without_initial_value() const
    {
      return mandatory_no_initial_value_;
    }

    const char* field_type_name() const;

    field_instruction(uint16_t        field_index,
                      operator_enum_t operator_id,
                      int             field_type,
                      presence_enum_t optional,
                      uint32_t        id,
                      const char*     name,
                      const char*     ns)
      : field_index_(field_index)
      , operator_id_(operator_id)
      , optional_flag_(optional)
      , nullable_flag_( optional &&  (operator_id != operator_constant) )
      , has_pmap_bit_(operator_id > operator_delta || ((operator_id == operator_constant) && optional))
      , mandatory_no_initial_value_(false)
      , field_type_(field_type)
      , id_(id)
      , name_(name)
      , ns_(ns)
    {
    }

  protected:

    uint16_t field_index_;
    uint16_t operator_id_ : 4;
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

    const value_storage& initial_value() const
    {
      return initial_value_;
    }

    const value_storage& initial_or_default_value() const
    {
      return *initial_or_default_value_;
    }

  protected:

    friend class dictionary_builder;
    const op_context_t* op_context_;
    value_storage initial_value_;
    value_storage* prev_value_;
    value_storage prev_storage_;
    const value_storage* initial_or_default_value_;
    static const value_storage default_value_;
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

};

// #if !defined(BOOST_NO_CXX11_EXTERN_TEMPLATE) && !defined(MFAST_DYN_LINK) && !defined(mfast_EXPORTS)
// extern template class int_field_instruction<int32_t>;
// extern template class int_field_instruction<uint32_t>;
// extern template class int_field_instruction<int64_t>;
// extern template class int_field_instruction<uint64_t>;
// #elif defined(MFAST_DYN_LINK) || defined(mfast_EXPORTS)
// template class MFAST_EXPORT int_field_instruction<int32_t>;
// template class MFAST_EXPORT int_field_instruction<uint32_t>;
// template class MFAST_EXPORT int_field_instruction<int64_t>;
// template class MFAST_EXPORT int_field_instruction<uint64_t>;
// #endif

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
                              mantissa_field_instruction* mantissa_instruction,
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
      , mantissa_instruction_(mantissa_instruction)
    {
      assert(mantissa_instruction);
      this->initial_value_.of_decimal.mantissa_ = mantissa_instruction->initial_value().get<int64_t>();

      if (has_pmap_bit_ == 0) {
        has_pmap_bit_ = mantissa_instruction->pmap_size();
      }
    }

    decimal_field_instruction(const decimal_field_instruction& other,
                              mantissa_field_instruction*      mantissa_instruction)
      : integer_field_instruction_base(other)
      , mantissa_instruction_(mantissa_instruction)
    {
    }

    /// Perform deep copy
    virtual void copy_value(const value_storage& src,
                            value_storage&       dest,
                            allocator*           alloc) const;


    virtual void construct_value(value_storage& storage,
                                 allocator*     alloc) const;


    virtual void accept(field_instruction_visitor&, void*) const;

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

  protected:

    friend class dictionary_builder;
    mantissa_field_instruction* mantissa_instruction_;
};


class MFAST_EXPORT string_field_instruction
  : public field_instruction
{
  public:

    string_field_instruction(uint16_t             field_index,
                             operator_enum_t      operator_id,
                             field_type_enum_t    field_type,
                             presence_enum_t      optional,
                             uint32_t             id,
                             const char*          name,
                             const char*          ns,
                             const op_context_t*  context,
                             string_value_storage initial_value)
      : field_instruction(field_index, operator_id, field_type, optional,
                          id,
                          name,
                          ns)
      , op_context_(context)
      , initial_value_(initial_value.storage_)
      , prev_value_(&prev_storage_)
      , initial_or_default_value_(initial_value_.is_empty() ? &default_value_ : &initial_value_)
    {
      mandatory_no_initial_value_ = !optional && initial_value.storage_.is_empty();
    }

    string_field_instruction(const string_field_instruction& other)
      : field_instruction(other)
      , op_context_(other.op_context_)
      , initial_value_(other.initial_value_)
      , prev_value_(&prev_storage_)
      , initial_or_default_value_(initial_value_.is_empty() ? &default_value_ : &initial_value_)
    {
    }

    virtual void construct_value(value_storage& storage,
                                 allocator*     alloc) const;
    virtual void destruct_value(value_storage& storage,
                                allocator*     alloc) const;


    // perform deep copy
    virtual void copy_value(const value_storage& src,
                            value_storage&       dest,
                            allocator*           alloc) const;


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

    const value_storage& initial_value() const
    {
      return initial_value_;
    }

    const value_storage& initial_or_default_value() const
    {
      return *initial_or_default_value_;
    }

  protected:
    friend class dictionary_builder;
    const op_context_t* op_context_;
    value_storage initial_value_;
    value_storage* prev_value_;
    value_storage prev_storage_;
    const value_storage* initial_or_default_value_;
    static const value_storage default_value_;
};


class MFAST_EXPORT ascii_field_instruction
  : public string_field_instruction
{
  public:
    ascii_field_instruction(uint16_t             field_index,
                            operator_enum_t      operator_id,
                            presence_enum_t      optional,
                            uint32_t             id,
                            const char*          name,
                            const char*          ns,
                            const op_context_t*  context,
                            string_value_storage initial_value = string_value_storage())
      : string_field_instruction(field_index,
                                 operator_id,
                                 field_type_ascii_string,
                                 optional,
                                 id, name, ns, context,
                                 initial_value)
    {
    }

    ascii_field_instruction(const ascii_field_instruction& other)
      : string_field_instruction(other)
    {
    }

    virtual void accept(field_instruction_visitor& visitor, void* context) const;
};

class MFAST_EXPORT unicode_field_instruction
  : public string_field_instruction
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
                              const char*          length_ns = "")
      : string_field_instruction(field_index,
                                 operator_id,
                                 field_type_unicode_string,
                                 optional,
                                 id, name, ns, context, initial_value)
      , length_id_(length_id)
      , length_name_(length_name)
      , length_ns_(length_ns)
    {
    }

    unicode_field_instruction(const unicode_field_instruction& other)
      : string_field_instruction(other)
      , length_id_(other.length_id_)
      , length_name_(other.length_name_)
      , length_ns_(other.length_ns_)
    {
    }

    virtual void accept(field_instruction_visitor& visitor, void* context) const;
    
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

  protected:
    uint32_t length_id_;
    const char* length_name_;
    const char* length_ns_;
};



class MFAST_EXPORT byte_vector_field_instruction
  : public string_field_instruction
{
  public:
    byte_vector_field_instruction(uint16_t                  field_index,
                                  operator_enum_t           operator_id,
                                  presence_enum_t           optional,
                                  uint32_t                  id,
                                  const char*               name,
                                  const char*               ns,
                                  const op_context_t*       value_context,
                                  byte_vector_value_storage initial_value = byte_vector_value_storage(),
                                  uint32_t                  length_id = 0,
                                  const char*               length_name = "",
                                  const char*               length_ns = "")
      : string_field_instruction(field_index,
                                 operator_id,
                                 field_type_byte_vector,
                                 optional,
                                 id, name, ns, value_context,
                                 initial_value)
      , length_id_(length_id)
      , length_name_(length_name)
      , length_ns_(length_ns)
    {
    }

    byte_vector_field_instruction(const byte_vector_field_instruction& other)
      : string_field_instruction(other)
      , length_id_(other.length_id_)
      , length_name_(other.length_name_)
      , length_ns_(other.length_ns_)
    {
    }

    virtual void accept(field_instruction_visitor& visitor, void* context) const;

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
  protected:
    uint32_t length_id_;
    const char* length_name_;
    const char* length_ns_;
};

struct MFAST_EXPORT aggregate_instruction_base
{
  aggregate_instruction_base(const char* dictionary,
                             void*       subinstructions,
                             uint32_t    subinstructions_count,
                             const char* typeref_name,
                             const char* typeref_ns)
    : dictionary_(dictionary)
    , subinstructions_count_(subinstructions_count)
    , typeref_name_(typeref_name)
    , typeref_ns_(typeref_ns)
    , segment_pmap_size_(0)
  {
    set_subinstructions(static_cast<field_instruction**>(subinstructions));
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

  field_instruction* subinstruction(std::size_t index) const
  {
    assert(index < subinstructions_count_);
    return subinstructions_[index];
  }

  std::size_t segment_pmap_size() const
  {
    return segment_pmap_size_;
  }

  void set_subinstructions(field_instruction** subinstructions)
  {
    subinstructions_ = subinstructions;
    segment_pmap_size_ = 0;
    for (uint32_t i = 0; i < subinstructions_count_; ++i) {
      segment_pmap_size_ += subinstruction(i)->pmap_size();
    }
  }

  const char* dictionary_;
  uint32_t subinstructions_count_;
  const char* typeref_name_;
  const char* typeref_ns_;
  std::size_t segment_pmap_size_;

  private:
    field_instruction** subinstructions_;
};

class MFAST_EXPORT group_field_instruction
  : public field_instruction
  , public aggregate_instruction_base
{
  public:

    group_field_instruction(uint16_t        field_index,
                            presence_enum_t optional,
                            uint32_t        id,
                            const char*     name,
                            const char*     ns,
                            const char*     dictionary,
                            void*           subinstructions,
                            uint32_t        subinstructions_count,
                            const char*     typeref_name ="",
                            const char*     typeref_ns="")
      : field_instruction(field_index,
                          operator_constant,
                          field_type_group,
                          optional,
                          id,
                          name, ns)
      , aggregate_instruction_base(dictionary, subinstructions,
                                   subinstructions_count,
                                   typeref_name,
                                   typeref_ns)
    {
      has_pmap_bit_ = segment_pmap_size() > 0 ? 1 : 0;
    }

    virtual void construct_value(value_storage& storage,
                                 allocator*     alloc) const;
    virtual void destruct_value(value_storage& storage,
                                allocator*     alloc) const;

    // perform deep copy
    virtual void copy_value(const value_storage& src,
                            value_storage&       dest,
                            allocator*           alloc) const;

    virtual void accept(field_instruction_visitor&, void*) const;


    void ensure_valid_storage(value_storage& storage,
                              allocator*     alloc) const;
};

template <typename T>
class group_instruction_ex
  : public group_field_instruction
{
  public:
    group_instruction_ex(uint16_t        field_index,
                         presence_enum_t optional,
                         uint32_t        id,
                         const char*     name,
                         const char*     ns,
                         const char*     dictionary,
                         void*           subinstructions,
                         uint32_t        subinstructions_count,
                         const char*     typeref_name ="",
                         const char*     typeref_ns="")
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

};

class MFAST_EXPORT sequence_field_instruction
  : public field_instruction
  , public aggregate_instruction_base
{
  public:
    sequence_field_instruction(uint16_t                  field_index,
                               presence_enum_t           optional,
                               uint32_t                  id,
                               const char*               name,
                               const char*               ns,
                               const char*               dictionary,
                               void*                     subinstructions,
                               uint32_t                  subinstructions_count,
                               uint32_field_instruction* sequence_length_instruction,
                               const char*               typeref_name="",
                               const char*               typeref_ns="")
      : field_instruction(field_index,
                          operator_constant,
                          field_type_sequence,
                          optional,
                          id,
                          name, ns)
      , aggregate_instruction_base(dictionary,
                                   subinstructions,
                                   subinstructions_count,
                                   typeref_name,
                                   typeref_ns)
      , sequence_length_instruction_(sequence_length_instruction)
    {
      has_pmap_bit_ = segment_pmap_size() > 0 ? 1 : 0;
    }

    virtual void construct_value(value_storage& storage,
                                 allocator*     alloc) const;
    virtual void destruct_value(value_storage& storage,
                                allocator*     alloc) const;

    void construct_sequence_elements(value_storage& storage,
                                     std::size_t    start,
                                     std::size_t    length,
                                     allocator*     alloc) const;
    void destruct_sequence_elements(value_storage& storage,
                                    std::size_t    start,
                                    std::size_t    length,
                                    allocator*     alloc) const;

    // perform deep copy
    virtual void copy_value(const value_storage& src,
                            value_storage&       dest,
                            allocator*           alloc) const;

    virtual void accept(field_instruction_visitor&, void*) const;
    const uint32_field_instruction* length_instruction() const
    {
      return sequence_length_instruction_;
    }

  private:
    friend class dictionary_builder;
    uint32_field_instruction* sequence_length_instruction_;
};

template <typename T>
class sequence_instruction_ex
  : public sequence_field_instruction
{
  public:
    sequence_instruction_ex(uint16_t                  field_index,
                            presence_enum_t           optional,
                            uint32_t                  id,
                            const char*               name,
                            const char*               ns,
                            const char*               dictionary,
                            void*                     subinstructions,
                            uint32_t                  subinstructions_count,
                            uint32_field_instruction* sequence_length_instruction,
                            const char*               typeref_name="",
                            const char*               typeref_ns="")
      : sequence_field_instruction(field_index, optional, id, name, ns, dictionary, subinstructions,
                                   subinstructions_count, sequence_length_instruction, typeref_name, typeref_ns)
    {
    }

};

class MFAST_EXPORT template_instruction
  : public group_field_instruction
{
  public:
    template_instruction(uint32_t    id,
                         const char* name,
                         const char* ns,
                         const char* template_ns,
                         const char* dictionary,
                         void*       subinstructions,
                         uint32_t    subinstructions_count,
                         bool        reset,
                         const char* typeref_name="",
                         const char* typeref_ns="")
      : group_field_instruction(0, presence_mandatory,
                                id,
                                name,
                                ns,
                                dictionary,
                                subinstructions,
                                subinstructions_count,
                                typeref_name,
                                typeref_ns)
      , template_ns_(template_ns)
      , reset_(reset)
    {
      field_type_ = field_type_template;
    }

    const char* template_ns() const
    {
      return template_ns_;
    }

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Woverloaded-virtual"
#endif

    void construct_value(value_storage& storage,
                         value_storage* fields_storage,
                         allocator*     alloc,
                         bool           construct_subfields=true) const;


    void copy_construct_value(value_storage&       storage,
                              value_storage*       fields_storage,
                              allocator*           alloc,
                              const value_storage* src) const;

#ifdef __clang__
#pragma clang diagnostic pop
#endif

    virtual void accept(field_instruction_visitor&, void*) const;

    bool has_reset_attribute() const
    {
      return reset_;
    }

  private:
    const char* template_ns_;
    bool reset_;
};

template <typename T>
class template_instruction_ex
  : public template_instruction
{
  public:
    template_instruction_ex(uint32_t    id,
                            const char* name,
                            const char* ns,
                            const char* template_ns,
                            const char* dictionary,
                            void*       subinstructions,
                            uint32_t    subinstructions_count,
                            bool        reset,
                            const char* typeref_name="",
                            const char* typeref_ns="")
      : template_instruction(id, name, ns, template_ns, dictionary,
                             subinstructions, subinstructions_count, reset, typeref_name, typeref_ns)
    {
    }

};

class MFAST_EXPORT templateref_instruction
  : public field_instruction
{
  public:

    templateref_instruction(uint16_t    field_index,
                            const char* name = "",
                            const char* ns = "")
      : field_instruction(field_index, operator_none, field_type_templateref, presence_mandatory, 0, name, ns)
      , target_(0)
    {
      // I used empty string instead of null pointer for name to represent dynamic templateRef because I wanted
      // to be able to print the name of dynamic templateRef directly (albeit empty) without using an if branch.
    }

    templateref_instruction(uint16_t                    field_index,
                            const template_instruction* ref)
      : field_instruction(field_index, operator_none, field_type_templateref, presence_mandatory, 0, ref->name(), ref->ns())
      , target_(ref)
    {
    }

    virtual void construct_value(value_storage& storage,
                                 allocator*     alloc) const;
    virtual void destruct_value(value_storage& storage,
                                allocator*     alloc) const;

    virtual std::size_t pmap_size() const;

    /// Perform deep copy
    virtual void copy_value(const value_storage& src,
                            value_storage&       dest,
                            allocator*           alloc) const;

    virtual void accept(field_instruction_visitor&, void*) const;

    const template_instruction* target() const
    {
      return target_;
    }

    bool is_static() const
    {
      return name()[0] != 0;
    }

  private:
    const template_instruction* target_;
};

class templates_loader;
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

    friend class templates_loader;
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
};

template <typename T>
void int_field_instruction<T>::accept(field_instruction_visitor& visitor, void* context) const
{
  visitor.visit(this, context);
}

template <typename T, bool IsVectorOrAscii=false>
struct instruction_trait;


template <>
struct instruction_trait<int32_t, false>
{
  typedef int32_field_instruction type;
};

template <>
struct instruction_trait<uint32_t, false>
{
  typedef uint32_field_instruction type;
};

template <>
struct instruction_trait<int64_t, false>
{
  typedef int64_field_instruction type;
};

template <>
struct instruction_trait<uint64_t, false>
{
  typedef uint64_field_instruction type;
};

template <>
struct instruction_trait<char, true>
{
  typedef ascii_field_instruction type;
};

template <>
struct instruction_trait<char, false>
{
  typedef unicode_field_instruction type;
};

template <>
struct instruction_trait<unsigned char, true>
{
  typedef byte_vector_field_instruction type;
};


} // namespace mfast
#endif /* end of include guard: FIELD_INSTRUCTION_H_PMKVDZOC */
