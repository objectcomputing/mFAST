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
#ifndef TYPE_DEFINITION_H_PMKVDZOC
#define TYPE_DEFINITION_H_PMKVDZOC

#include <cstddef>
#include <cassert>
#include <boost/static_assert.hpp>
#include "mfast/value_storage.h"
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

template <class T>
class nullable
{
  public:
    nullable()
      : is_null_(true)
    {
    }

    nullable(T value)
      :  v_(value)
      , is_null_(false)
    {
    }

    void set(T value)
    {
      v_ = value;
      is_null_ = false;
    }

    bool is_null() const
    {
      return is_null_;
    }

    T value() const
    {
      return v_;
    }

  private:
    T v_;
    bool is_null_;
};


class field_instruction
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
    bool is_nullable() const
    {
      return nullable_flag_;
    }

    bool optional() const
    {
      return optional_flag_;
    }

    bool has_initial_value() const
    {
      return has_initial_value_;
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

    bool has_pmap_bit() const
    {
      return has_pmap_bit_;
    }

    operator_enum_t field_operator() const
    {
      return static_cast<operator_enum_t>(operator_id_);
    }

    field_instruction(operator_enum_t operator_id,
                      int             field_type,
                      presence_enum_t optional,
                      uint32_t        id,
                      const char*     name,
                      const char*     ns)
      : operator_id_(operator_id)
      , optional_flag_(optional)
      , nullable_flag_( optional &&  (operator_id != operator_constant) )
      , has_pmap_bit_(operator_id > operator_delta ||
                      ((operator_id == operator_constant) && optional))
      , field_type_(field_type)
      , id_(id)
      , name_(name)
      , ns_(ns)
    {
    }

  protected:
    uint16_t operator_id_ : 4;
    uint16_t optional_flag_ : 1;
    uint16_t nullable_flag_ : 1;
    uint16_t has_initial_value_ : 1;
    uint16_t has_pmap_bit_ : 1;
    uint16_t field_type_;
    uint32_t id_;
    const char* name_;
    const char* ns_;
};

class dictionary_builder;

class integer_field_instruction_base
  : public field_instruction
{
  public:
    integer_field_instruction_base(operator_enum_t     operator_id,
                                   int                 field_type,
                                   presence_enum_t     optional,
                                   uint32_t            id,
                                   const char*         name,
                                   const char*         ns,
                                   const op_context_t* context)
      : field_instruction(operator_id, field_type, optional, id, name, ns)
      , op_context_(context)
      , default_value_(1)
      , prev_value_(&prev_storage_)
    {
    }

    integer_field_instruction_base(const integer_field_instruction_base& other)
      : field_instruction(other)
      , op_context_(other.op_context_)
      , default_value_(other.default_value_)
      , prev_value_(&prev_storage_)
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

    const value_storage& default_value() const
    {
      return default_value_;
    }

  protected:

    friend class dictionary_builder;
    const op_context_t* op_context_;
    value_storage default_value_;
    value_storage* prev_value_;
    value_storage prev_storage_;
};

template <typename T>
class int_field_instruction
  : public integer_field_instruction_base
{
  public:
    int_field_instruction(operator_enum_t     operator_id,
                          presence_enum_t     optional,
                          uint32_t            id,
                          const char*         name,
                          const char*         ns,
                          const op_context_t* context,
                          nullable<T>         initial_value = nullable<T>())
      : integer_field_instruction_base(operator_id,
                                       field_type_trait<T>::id,
                                       optional,
                                       id,
                                       name,
                                       ns,
                                       context)
    {
      this->has_initial_value_ = !initial_value.is_null();
      if (this->has_initial_value_)
        reinterpret_cast<T&>(this->default_value_.of_uint.content_) = initial_value.value();
    }

    int_field_instruction(const int_field_instruction& other)
      : integer_field_instruction_base(other)
    {
    }

    virtual void accept(field_instruction_visitor& visitor, void* context) const;

    T initial_value() const
    {
      return *reinterpret_cast<const T*>(&this->default_value_.of_uint.content_);
    }

};

typedef int_field_instruction<int32_t> int32_field_instruction;
typedef int_field_instruction<uint32_t> uint32_field_instruction;
typedef int_field_instruction<int64_t> int64_field_instruction;
typedef int_field_instruction<uint64_t> uint64_field_instruction;

class mantissa_field_instruction
  : public int64_field_instruction
{
  public:

    mantissa_field_instruction(operator_enum_t     operator_id,
                               const op_context_t* context,
                               nullable<int64_t>   initial_value)
      : int64_field_instruction(operator_id, presence_mandatory, 0, 0, 0, context, initial_value)
    {
    }

    mantissa_field_instruction(const mantissa_field_instruction& other)
      : int64_field_instruction(other)
    {
    }

};


class nullable_decimal
{
  public:
    nullable_decimal()
      : is_null_ (true)
    {
    }

    nullable_decimal(int64_t mantissa, int8_t exponent)
      : mantissa_(mantissa)
      , exponent_(exponent)
      , is_null_(false)
    {
    }

    void set(int64_t mantissa, int8_t exponent)
    {
      mantissa_ = mantissa;
      exponent_ = exponent;
      is_null_ = false;
    }

    bool is_null() const
    {
      return is_null_;
    }

    int64_t mantissa() const
    {
      return mantissa_;
    }

    int8_t exponent() const
    {
      return exponent_;
    }

  private:
    int64_t mantissa_;
    int8_t exponent_;
    bool is_null_;
};

class decimal_field_instruction
  : public integer_field_instruction_base
{
  public:

    decimal_field_instruction(operator_enum_t     decimal_operator_id,
                              presence_enum_t     optional,
                              uint32_t            id,
                              const char*         name,
                              const char*         ns,
                              const op_context_t* decimal_context,
                              nullable_decimal    initial_value = nullable_decimal())
      : integer_field_instruction_base(decimal_operator_id,
                                       field_type_decimal,
                                       optional,
                                       id,
                                       name,
                                       ns,
                                       decimal_context)
      , mantissa_instruction_(0)
    {
      this->has_initial_value_ = !initial_value.is_null();
      if (this->has_initial_value_) {
        this->default_value_.of_decimal.exponent_ = initial_value.exponent();
        this->default_value_.of_decimal.mantissa_ = initial_value.mantissa();
      }
    }

    decimal_field_instruction(operator_enum_t             exponent_operator_id,
                              presence_enum_t             optional,
                              uint32_t                    id,
                              const char*                 name,
                              const char*                 ns,
                              const op_context_t*         exponent_context,
                              mantissa_field_instruction* mantissa_instruction,
                              nullable<int8_t>            exponent_initial_value = nullable<int8_t>())
      : integer_field_instruction_base(exponent_operator_id,
                                       field_type_exponent,
                                       optional,
                                       id,
                                       name,
                                       ns,
                                       exponent_context)
      , mantissa_instruction_(mantissa_instruction)
    {
      assert(mantissa_instruction);
      this->has_initial_value_ = !exponent_initial_value.is_null();
      if (this->has_initial_value_ ) {
        this->default_value_.of_decimal.exponent_ = exponent_initial_value.value();
        this->default_value_.of_decimal.mantissa_ = mantissa_instruction->initial_value();
      }

      if (!has_pmap_bit_) {
        has_pmap_bit_ = mantissa_instruction->has_pmap_bit();
      }
    }

    decimal_field_instruction(const decimal_field_instruction& other,
                              mantissa_field_instruction*      mantissa_instruction)
      : integer_field_instruction_base(other)
      , mantissa_instruction_(mantissa_instruction)
    {
    }

    int64_t mantissa_initial_value() const
    {
      return this->default_value_.of_decimal.mantissa_;
    }

    int8_t exponent_initial_value() const
    {
      return static_cast<int8_t>(this->default_value_.of_decimal.exponent_);
    }

    virtual void construct_value(value_storage& storage,
                                 allocator*     alloc) const;


    virtual void accept(field_instruction_visitor&, void*) const;

    const mantissa_field_instruction* mantissa_instruction() const
    {
      return mantissa_instruction_;
    }

  protected:

    friend class dictionary_builder;
    mantissa_field_instruction* mantissa_instruction_;
};


class string_field_instruction
  : public field_instruction
{
  public:

    string_field_instruction(operator_enum_t     operator_id,
                             field_type_enum_t   field_type,
                             presence_enum_t     optional,
                             uint32_t            id,
                             const char*         name,
                             const char*         ns,
                             const op_context_t* context,
                             const char*         initial_value=0,
                             uint32_t            initial_value_length=0)
      : field_instruction(operator_id, field_type, optional,
                          id,
                          name,
                          ns)
      , op_context_(context)
      , default_value_(1)
      , prev_value_(&prev_storage_)
    {
      this->has_initial_value_ = (initial_value != 0);
      if (initial_value) {
        this->has_initial_value_ = 1;
        this->default_value_.of_array.content_ = const_cast<char*>(initial_value);
        this->default_value_.array_length(initial_value_length);
      }
      else {
        this->has_initial_value_ = 0;
        this->default_value_.of_array.content_ = const_cast<char*>("");
        this->default_value_.array_length(0);
      }
    }

    string_field_instruction(const string_field_instruction& other)
      : field_instruction(other)
      , op_context_(other.op_context_)
      , default_value_(other.default_value_)
      , prev_value_(&prev_storage_)
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

    const value_storage& default_value() const
    {
      return default_value_;
    }

  protected:
    friend class dictionary_builder;
    const op_context_t* op_context_;
    value_storage default_value_;
    value_storage* prev_value_;
    value_storage prev_storage_;
};


class ascii_field_instruction
  : public string_field_instruction
{
  public:
    ascii_field_instruction(operator_enum_t     operator_id,
                            presence_enum_t     optional,
                            uint32_t            id,
                            const char*         name,
                            const char*         ns,
                            const op_context_t* context,
                            const char*         initial_value=0,
                            uint32_t            initial_value_length=0)
      : string_field_instruction(operator_id,
                                 field_type_ascii_string,
                                 optional,
                                 id, name, ns, context, initial_value,
                                 initial_value_length)
    {
    }

    ascii_field_instruction(const ascii_field_instruction& other)
      : string_field_instruction(other)
    {
    }

    virtual void accept(field_instruction_visitor& visitor, void* context) const;
};

class unicode_field_instruction
  : public string_field_instruction
{
  public:
    unicode_field_instruction(operator_enum_t     operator_id,
                              presence_enum_t     optional,
                              uint32_t            id,
                              const char*         name,
                              const char*         ns,
                              const op_context_t* context,
                              const char*         initial_value=0,
                              uint32_t            initial_value_length=0)
      : string_field_instruction(operator_id,
                                 field_type_unicode_string,
                                 optional,
                                 id, name, ns, context, initial_value,
                                 initial_value_length)
    {
    }

    unicode_field_instruction(const unicode_field_instruction& other)
      : string_field_instruction(other)
    {
    }

    virtual void accept(field_instruction_visitor& visitor, void* context) const;
};



class byte_vector_field_instruction
  : public string_field_instruction
{
  public:
    byte_vector_field_instruction(operator_enum_t     operator_id,
                                  presence_enum_t     optional,
                                  uint32_t            id,
                                  const char*         name,
                                  const char*         ns,
                                  const op_context_t* value_context,
                                  uint32_t            length_id,
                                  const char*         length_name,
                                  const char*         length_ns)
      : string_field_instruction(operator_id,
                                 field_type_byte_vector,
                                 optional,
                                 id, name, ns, value_context)
      , length_id_(length_id)
      , length_name_(length_name)
      , length_ns_(length_ns)
    {
    }

    byte_vector_field_instruction(operator_enum_t      operator_id,
                                  presence_enum_t      optional,
                                  uint32_t             id,
                                  const char*          name,
                                  const char*          ns,
                                  const op_context_t*  value_context,
                                  const unsigned char* initial_value,
                                  uint32_t             initial_value_length,
                                  uint32_t             length_id,
                                  const char*          length_name,
                                  const char*          length_ns)
      : string_field_instruction(operator_id,
                                 field_type_byte_vector,
                                 optional,
                                 id, name, ns, value_context,
                                 reinterpret_cast<const char*>(initial_value),
                                 initial_value_length)
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

  protected:
    uint32_t length_id_;
    const char* length_name_;
    const char* length_ns_;
};

struct group_content_helper
{
  group_content_helper(const char* dictionary,
                       void*       subinstructions,
                       uint32_t    subinstructions_count,
                       const char* typeref_name,
                       const char* typeref_ns)
    : dictionary_(dictionary),subinstructions_(static_cast<field_instruction**>(subinstructions))
    , subinstructions_count_(subinstructions_count)
    , typeref_name_(typeref_name)
    , typeref_ns_(typeref_ns)
  {
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

  bool subinstruction_has_pmap_bit() const
  {
    for (uint32_t i = 0; i < subinstructions_count_; ++i) {
      if (subinstruction(i)->has_pmap_bit())
        return true;
    }
    return false;
  }
  
  std::size_t pmap_size() const 
  {
    std::size_t n = 0;
    for (uint32_t i = 0; i < subinstructions_count_; ++i) {
      if (subinstruction(i)->has_pmap_bit())
        ++n;
    }
    return n;
  }

  const char* dictionary_;
  field_instruction** subinstructions_;
  uint32_t subinstructions_count_;
  const char* typeref_name_;
  const char* typeref_ns_;
};

class group_field_instruction
  : public field_instruction
  , public group_content_helper
{
  public:

    group_field_instruction(presence_enum_t optional,
                            uint32_t        id,
                            const char*     name,
                            const char*     ns,
                            const char*     dictionary,
                            void*           subinstructions,
                            uint32_t        subinstructions_count,
                            const char*     typeref_name ="",
                            const char*     typeref_ns="")
      : field_instruction(operator_constant, field_type_group, optional,
                          id,
                          name, ns)
      , group_content_helper(dictionary, subinstructions,
                             subinstructions_count,
                             typeref_name,
                             typeref_ns)
    {
      has_pmap_bit_ = subinstruction_has_pmap_bit();
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
    group_instruction_ex(presence_enum_t optional,
                         uint32_t        id,
                         const char*     name,
                         const char*     ns,
                         const char*     dictionary,
                         void*           subinstructions,
                         uint32_t        subinstructions_count,
                         const char*     typeref_name ="",
                         const char*     typeref_ns="")
      : group_field_instruction(optional, id, name, ns, dictionary, subinstructions, subinstructions_count, typeref_name, typeref_ns)
    {
    }

};

class sequence_field_instruction
  : public field_instruction
  , public group_content_helper
{
  public:
    sequence_field_instruction(presence_enum_t           optional,
                               uint32_t                  id,
                               const char*               name,
                               const char*               ns,
                               const char*               dictionary,
                               void*                     subinstructions,
                               uint32_t                  subinstructions_count,
                               uint32_field_instruction* sequence_length_instruction,
                               const char*               typeref_name="",
                               const char*               typeref_ns="")
      : field_instruction(operator_constant, field_type_sequence, optional,
                          id,
                          name, ns)
      , group_content_helper(dictionary,
                             subinstructions,
                             subinstructions_count,
                             typeref_name,
                             typeref_ns)
      , sequence_length_instruction_(sequence_length_instruction)
    {
      has_pmap_bit_ = subinstruction_has_pmap_bit();
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


    uint32_field_instruction* sequence_length_instruction_;
};

template <typename T>
class sequence_instruction_ex
  : public sequence_field_instruction
{
  public:
    sequence_instruction_ex(presence_enum_t           optional,
                            uint32_t                  id,
                            const char*               name,
                            const char*               ns,
                            const char*               dictionary,
                            void*                     subinstructions,
                            uint32_t                  subinstructions_count,
                            uint32_field_instruction* sequence_length_instruction,
                            const char*               typeref_name="",
                            const char*               typeref_ns="")
      : sequence_field_instruction(optional, id, name, ns, dictionary, subinstructions,
                                   subinstructions_count, sequence_length_instruction, typeref_name, typeref_ns)
    {
    }

};

class template_instruction
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
      : group_field_instruction(presence_mandatory,
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
      has_pmap_bit_ = true;
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

class templateref_instruction
  : public field_instruction
{
  public:
    templateref_instruction(const char* name,
                            const char* ns)
      : field_instruction(operator_none, field_type_templateref, presence_mandatory, 0, name, ns)
    {
    }

    virtual void construct_value(value_storage& storage,
                                 allocator*     alloc) const;
    virtual void destruct_value(value_storage& storage,
                                allocator*     alloc) const;


    /// Perform deep copy
    virtual void copy_value(const value_storage& src,
                            value_storage&       dest,
                            allocator*           alloc) const;

    virtual void accept(field_instruction_visitor&, void*) const;


    static templateref_instruction* instance();
};

class templates_loader;
class templates_description
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

    const template_instruction* instruction(std::size_t i) const
    {
      return instructions_[i];
    }

    uint32_t instructions_count() const
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


class field_instruction_visitor
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
#endif /* end of include guard: TYPE_DEFINITION_H_PMKVDZOC */
