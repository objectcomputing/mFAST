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
#include <cstring>
#include "mfast/field_instruction.h"
#include "mfast/allocator.h"

namespace mfast {

field_instruction::field_instruction(uint16_t        field_index,
                                     operator_enum_t operator_id,
                                     int             field_type,
                                     presence_enum_t optional,
                                     uint32_t        id,
                                     const char*     name,
                                     const char*     ns)
  : field_index_(field_index)
  , operator_id_(operator_id)
  , is_array_(field_type >= field_type_ascii_string && field_type <= field_type_sequence )
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

void
field_instruction::destruct_value(value_storage&,
                                  allocator*) const
{
}

void
field_instruction::copy_value(const value_storage& src,
                              value_storage&       dest,
                              allocator* /* alloc */) const
{
  dest.of_array.content_ = src.of_array.content_;
  dest.of_array.len_ = src.of_array.len_;
  dest.of_array.capacity_ = 0;
}

const char*
field_instruction::field_type_name() const
{
  static const char* names [] = {
    "field_type_int32",
    "field_type_uint32",
    "field_type_int64",
    "field_type_uint64",
    "field_type_decimal",
    "field_type_exponent",
    "field_type_ascii_string",
    "field_type_unicode_string",
    "field_type_byte_vector",
    "field_type_group",
    "field_type_sequence",
    "field_type_templateref",
    "field_type_template"
  };
  return names[this->field_type()];
}

std::size_t
field_instruction::pmap_size() const
{
  return has_pmap_bit_;
}

//////////////////////////////////////////////////////

void integer_field_instruction_base::construct_value(value_storage& storage,
                                                     allocator* /* alloc */) const
{
  storage.of_uint.content_ = initial_value_.of_uint.content_;
  storage.of_uint.defined_bit_ = 1;
  storage.of_uint.present_ = !optional();
}

const value_storage integer_field_instruction_base::default_value_(1);

// template class int_field_instruction<int32_t>;
// template class int_field_instruction<uint32_t>;
// template class int_field_instruction<int64_t>;
// template class int_field_instruction<uint64_t>;

/////////////////////////////////////////////////////////

void decimal_field_instruction::construct_value(value_storage& storage,
                                                allocator*       ) const
{
  storage = initial_value_;
  storage.of_decimal.present_ = !optional();
}

void
decimal_field_instruction::copy_value(const value_storage& src,
                                      value_storage&       dest,
                                      allocator* /* alloc */) const
{
  dest.of_decimal.present_ =  src.of_decimal.present_;
  dest.of_decimal.mantissa_ = src.of_decimal.mantissa_;
  dest.of_decimal.exponent_ = src.of_decimal.exponent_;
}

void decimal_field_instruction::accept(field_instruction_visitor& visitor,
                                       void*                      context) const
{
  visitor.visit(this, context);
}

/////////////////////////////////////////////////////////

void string_field_instruction::construct_value(value_storage& storage,
                                               allocator*       ) const
{
  storage = initial_value_;
  if (optional())
    storage.of_array.len_ = 0;
}

void string_field_instruction::destruct_value(value_storage& storage,
                                              allocator*     alloc) const
{
  if (storage.of_array.capacity_) {
    alloc->deallocate(storage.of_array.content_, storage.of_array.capacity_);
  }
}

void string_field_instruction::copy_value(const value_storage& src,
                                          value_storage&       dest,
                                          allocator*           alloc) const
{
  size_t len = src.of_array.len_;
  if (len && src.of_array.content_ != initial_value_.of_array.content_) {
    dest.of_array.content_ = alloc->allocate(len);
    memcpy(dest.of_array.content_, src.of_array.content_, len);
    dest.of_array.capacity_ = len;
  }
  else {
    dest.of_array.content_ = src.of_array.content_;
    dest.of_array.capacity_ = 0;
  }
  dest.of_array.len_ = len;
}

const value_storage string_field_instruction::default_value_("");

/////////////////////////////////////////////////////////

void ascii_field_instruction::accept(field_instruction_visitor& visitor,
                                     void*                      context) const
{
  visitor.visit(this, context);
}

void unicode_field_instruction::accept(field_instruction_visitor& visitor,
                                       void*                      context) const
{
  visitor.visit(this, context);
}

void byte_vector_field_instruction::accept(field_instruction_visitor& visitor,
                                           void*                      context) const
{
  visitor.visit(this, context);
}

/////////////////////////////////////////////////////////

void aggregate_instruction_base::construct_group_subfields(value_storage* subfields,
                                                           allocator*     alloc,
                                                           value_storage* parent) const
{
  for (uint32_t i = 0; i < this->subinstructions_count_; ++i) {
    this->subinstructions_[i]->construct_value(subfields[i], alloc);
  }
  if (parent) {
    subfields[subinstructions_count_].of_group.content_ = parent;
  }
}

void aggregate_instruction_base::destruct_group_subfields(value_storage* subfields,
                                                          allocator*     alloc) const
{
  for (uint32_t i = 0; i < this->subinstructions_count_; ++i) {
    this->subinstructions_[i]->destruct_value(subfields[i], alloc);
  }
}

int aggregate_instruction_base::find_subinstruction_index_by_id(uint32_t id) const
{
  for (uint32_t i = 0; i < this->subinstructions_count_; ++i) {
    if (this->subinstructions_[i]->id() == id)
      return i;
  }
  return -1;
}

int aggregate_instruction_base::find_subinstruction_index_by_name(const char* name) const
{
  for (uint32_t i = 0; i < this->subinstructions_count_; ++i) {
    if (std::strcmp(this->subinstructions_[i]->name(), name) ==0)
      return i;
  }
  return -1;
}

// deep copy
void aggregate_instruction_base::copy_group_subfields(const value_storage* src_subfields,
                                                      value_storage*       dest_subfields,
                                                      allocator*           alloc,
                                                      value_storage*       parent) const
{
  for (uint32_t i = 0; i < this->subinstructions_count_; ++i) {
    this->subinstructions_[i]->copy_value(src_subfields[i], dest_subfields[i], alloc);
  }

  if (parent) {
    dest_subfields[subinstructions_count_].of_group.content_ = parent;
  }
}

///////////////////////////////////////////////////////////////////

inline std::size_t
group_field_instruction::content_allocation_size() const
{
  std::size_t result = this->group_content_byte_count();
  if (optional()) {
    // for optional group, we need an extra storage used for storing the
    // pointer back to its parent so we have way to set the field as absent.
    result += sizeof(value_storage);
  }
  return result;
}

void group_field_instruction::construct_value(value_storage& storage,
                                              allocator*     alloc) const
{
  storage.of_group.present_ = !optional();

  // group field is never used for a dictionary key; so, we won't use this
  // function for reseting a key and thus no memory deallocation is required.
  storage.of_group.content_ =
    static_cast<value_storage*>(alloc->allocate( content_allocation_size() ));
  storage.of_group.own_content_ = true;
  construct_group_subfields(storage.of_group.content_,
                            alloc,
                            optional() ? &storage : 0);
}

void group_field_instruction::destruct_value(value_storage& storage,
                                             allocator*     alloc) const
{
  if (storage.of_group.content_) {
    destruct_group_subfields(storage.of_group.content_, alloc);
    if (storage.of_group.own_content_) {
      alloc->deallocate(storage.of_group.content_, content_allocation_size());
    }
  }
}

void group_field_instruction::copy_value(const value_storage& src,
                                         value_storage&       dest,
                                         allocator*           alloc) const
{
  dest.of_group.present_ = src.of_group.present_;
  dest.of_group.own_content_ = true;
  dest.of_group.content_ =
    static_cast<value_storage*>(alloc->allocate( content_allocation_size() ));

  copy_group_subfields(src.of_group.content_, dest.of_group.content_, alloc);
}

void group_field_instruction::accept(field_instruction_visitor& visitor,
                                     void*                      context) const
{
  visitor.visit(this, context);
}

/////////////////////////////////////////////////////////

void sequence_field_instruction::construct_sequence_elements(value_storage& storage,
                                                             std::size_t    start,
                                                             std::size_t    length,
                                                             allocator*     alloc) const
{
  value_storage* elements = static_cast<value_storage*>(storage.of_array.content_);
  for (std::size_t i = start; i < start+length; ++i ) {
    construct_group_subfields(&elements[i*subinstructions_count_], alloc);
  }
}

void sequence_field_instruction::destruct_sequence_elements(value_storage& storage,
                                                            std::size_t    start,
                                                            std::size_t    length,
                                                            allocator*     alloc) const
{
  value_storage* elements = static_cast<value_storage*>(storage.of_array.content_);
  for (std::size_t i = start; i < start+length; ++i ) {
    destruct_group_subfields(&elements[i*subinstructions_count_], alloc);
  }
}

void sequence_field_instruction::construct_value(value_storage& storage,
                                                 allocator*     alloc ) const
{
  // len_ == 0 is reserve for null/absent
  uint32_t initial_length = sequence_length_instruction_->initial_value().get<uint32_t>();
  storage.of_array.len_ = optional() ? 0 : initial_length+1;
  if (sequence_length_instruction_ && initial_length > 0) {
    std::size_t element_size = this->group_content_byte_count();
    std::size_t reserve_size = initial_length*element_size;
    storage.of_array.content_ = 0;
    storage.of_array.capacity_ =  alloc->reallocate(storage.of_array.content_, 0, reserve_size)/element_size;
    construct_sequence_elements(storage,0, initial_length, alloc);
  }
  else {
    storage.of_array.content_ = 0;
    storage.of_array.capacity_ = 0;
  }
}

void sequence_field_instruction::destruct_value(value_storage& storage,
                                                allocator*     alloc ) const
{
  if (storage.of_array.capacity_) {
    destruct_sequence_elements(storage, 0, storage.of_array.capacity_, alloc);
    alloc->deallocate(storage.of_array.content_, this->group_content_byte_count()*storage.of_array.capacity_ );
  }
}

void sequence_field_instruction::copy_value(const value_storage& src,
                                            value_storage&       dest,
                                            allocator*           alloc) const
{
  std::size_t size = src.of_array.len_;

  if (size > 0)
    --size;

  dest.of_array.len_ = src.of_array.len_;

  if (size) {
    std::size_t element_size = this->group_content_byte_count();
    std::size_t reserve_size = size*element_size;

    dest.of_array.content_ = 0;
    dest.of_array.capacity_ =  alloc->reallocate(dest.of_array.content_, 0, reserve_size)/element_size;

    const value_storage* src_elements = static_cast<const value_storage*>(src.of_array.content_);
    value_storage* dest_elements = static_cast<value_storage*>(dest.of_array.content_);

    for (std::size_t i = 0; i < size; ++i ) {
      std::size_t j = i* this->subinstructions_count_;
      copy_group_subfields(&src_elements[j], &dest_elements[j], alloc);
    }
  }
  else {
    dest.of_array.content_ = 0;
    dest.of_array.capacity_ = 0;
  }
}

void sequence_field_instruction::accept(field_instruction_visitor& visitor,
                                        void*                      context) const
{
  visitor.visit(this, context);
}

/////////////////////////////////////////////////////////



void template_instruction::construct_value(value_storage& storage,
                                           value_storage* fields_storage,
                                           allocator*     alloc,
                                           bool           construct_subfields) const
{
  if (fields_storage) {
    storage.of_group.own_content_ = false;
  }
  else {
    storage.of_group.own_content_ = true;
    fields_storage = static_cast<value_storage*>(
      alloc->allocate(this->group_content_byte_count()));
  }
  storage.of_group.content_ = fields_storage;

  if (construct_subfields)
    construct_group_subfields(fields_storage, alloc);
  else
    memset(fields_storage, 0, this->group_content_byte_count());
}

void template_instruction::copy_construct_value(value_storage&       storage,
                                                value_storage*       dest_fields_storage,
                                                allocator*           alloc,
                                                const value_storage* src_fields_storage) const
{
  if (dest_fields_storage) {
    storage.of_group.own_content_ = false;
  }
  else {
    storage.of_group.own_content_ = true;
    dest_fields_storage = static_cast<value_storage*>(
      alloc->allocate(this->group_content_byte_count()));
  }
  storage.of_group.content_ = dest_fields_storage;
  copy_group_subfields(src_fields_storage,
                       dest_fields_storage,
                       alloc);

}

void template_instruction::ensure_valid_storage(value_storage& storage,
                                                allocator*     alloc) const
{
  if (storage.of_group.content_ == 0) {
    // group field is never used for a dictionary key; so, we won't use this
    // function for reseting a key and thus no memory deallocation is required.
    storage.of_group.content_ =
      static_cast<value_storage*>(alloc->allocate( this->group_content_byte_count() ));
    memset(storage.of_group.content_, 0, this->group_content_byte_count());
  }
}

void template_instruction::accept(field_instruction_visitor& visitor,
                                  void*                      context) const
{
  visitor.visit(this, context);
}

///////////////////////////////////////////////////////////

void templateref_instruction::construct_value(value_storage& storage,
                                              allocator*     alloc) const
{
  storage.of_templateref.of_instruction.instruction_ = target_;
  if (target_) {
    storage.of_templateref.content_ = static_cast<value_storage*>(
      alloc->allocate(target_->group_content_byte_count()));

    target_->construct_group_subfields(storage.of_templateref.content_, alloc);
  }
  else {
    storage.of_templateref.content_ = 0;
  }
}

void templateref_instruction::destruct_value(value_storage& storage,
                                             allocator*     alloc) const
{
  if (storage.of_templateref.of_instruction.instruction_ && storage.of_templateref.content_) {
    storage.of_templateref.of_instruction.instruction_->destruct_group_subfields(
      static_cast<value_storage*>(storage.of_templateref.content_),
      alloc);
    alloc->deallocate(storage.of_templateref.content_, storage.of_templateref.of_instruction.instruction_->group_content_byte_count());
  }
}

void templateref_instruction::copy_value(const value_storage& src,
                                         value_storage&       dest,
                                         allocator*           alloc) const
{
  dest.of_templateref.of_instruction.instruction_ = src.of_templateref.of_instruction.instruction_;
  if (src.of_templateref.of_instruction.instruction_) {
    dest.of_templateref.content_ =
      static_cast<value_storage*>(alloc->allocate( dest.of_templateref.of_instruction.instruction_->group_content_byte_count() ));

    dest.of_templateref.of_instruction.instruction_->copy_group_subfields(
      src.of_templateref.content_,
      dest.of_templateref.content_,
      alloc);
  }
}

void templateref_instruction::accept(field_instruction_visitor& visitor,
                                     void*                      context) const
{
  visitor.visit(this, context);
}

std::size_t templateref_instruction::pmap_size() const
{
  if (target_) {
    return target_->segment_pmap_size();
  }
  return 0;
}

const const_instruction_ptr_t*
templateref_instruction::default_instructions()
{
  static const templateref_instruction templateref_instruction_signleton(0);
  static const field_instruction* array[] = {
    &templateref_instruction_signleton
  };
  return array;
}

}
