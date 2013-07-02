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

void
field_instruction::destruct_value(value_storage_t& storage,
                                  allocator*       alloc) const
{
}

void
field_instruction::copy_value(const value_storage_t& src,
                              value_storage_t&       dest,
                              allocator              * /* alloc */) const
{
  dest.array_storage.content_ = src.array_storage.content_;
  dest.array_storage.len_ = src.array_storage.len_;
  dest.array_storage.capacity_ = 0;
}

//////////////////////////////////////////////////////

void integer_field_instruction_base::construct_value(value_storage_t& storage,
                                                     allocator        * /* alloc */) const
{
  storage.uint_storage.content_ = default_value_.uint_storage.content_;
  storage.uint_storage.defined_bit_ = 1;
  storage.uint_storage.present_ = !optional();
}

/////////////////////////////////////////////////////////

void decimal_field_instruction::construct_value(value_storage_t& storage,
                                                allocator*       alloc) const
{
  storage = default_value_;
  storage.decimal_storage.present_ = !optional();
}

void decimal_field_instruction::accept(field_instruction_visitor& visitor,
                                       void*                      context) const
{
  visitor.visit(this, context);
}

/////////////////////////////////////////////////////////

void string_field_instruction::construct_value(value_storage_t& storage,
                                               allocator*       alloc) const
{
  storage = default_value_;
  if (optional())
    storage.array_storage.len_ = 0;
}

void string_field_instruction::destruct_value(value_storage_t& storage,
                                              allocator*       alloc) const
{
  if (storage.array_storage.capacity_) {
    alloc->deallocate(storage.array_storage.content_);
  }
}

void string_field_instruction::copy_value(const value_storage_t& src,
                                          value_storage_t&       dest,
                                          allocator*             alloc) const
{
  size_t len = src.array_storage.len_;
  if (len && src.array_storage.content_ != default_value_.array_storage.content_) {
    dest.array_storage.content_ = alloc->allocate(len);
    dest.array_storage.capacity_ = len;
  }
  else {
    dest.array_storage.content_ = src.array_storage.content_;
    dest.array_storage.capacity_ = 0;
  }
  dest.array_storage.len_ = len;
}

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

void group_content_helper::construct_group_subfields(value_storage_t* subfields,
                                                     allocator*       alloc) const
{
  for (uint32_t i = 0; i < this->subinstructions_count_; ++i) {
    this->subinstructions_[i]->construct_value(subfields[i], alloc);
  }
}

void group_content_helper::destruct_group_subfields(value_storage_t* subfields,
                                                    allocator*       alloc) const
{
  for (uint32_t i = 0; i < this->subinstructions_count_; ++i) {
    this->subinstructions_[i]->destruct_value(subfields[i], alloc);
  }
}

int group_content_helper::find_subinstruction_index_by_id(uint32_t id) const
{
  for (uint32_t i = 0; i < this->subinstructions_count_; ++i) {
    if (this->subinstructions_[i]->id() == id)
      return i;
  }
  return -1;
}

int group_content_helper::find_subinstruction_index_by_name(const char* name) const
{
  for (uint32_t i = 0; i < this->subinstructions_count_; ++i) {
    if (std::strcmp(this->subinstructions_[i]->name(), name) ==0)
      return i;
  }
  return -1;
}

// deep copy
void group_content_helper::copy_group_subfields(const value_storage_t* src_subfields,
                                                value_storage_t*       dest_subfields,
                                                allocator*             alloc) const
{
  for (uint32_t i = 0; i < this->subinstructions_count_; ++i) {
    this->subinstructions_[i]->copy_value(src_subfields[i], dest_subfields[i], alloc);
  }
}

///////////////////////////////////////////////////////////////////

void group_field_instruction::construct_value(value_storage_t& storage,
                                              allocator*       alloc ) const
{
  storage.group_storage.present_ = !optional();
  // group field is never used for a dictionary key; so, we won't use this
  // function for reseting a key and thus no memory deallocation is required.
  storage.group_storage.content_ =
    static_cast<value_storage_t*>(alloc->allocate( this->group_content_byte_count() ));

  construct_group_subfields(storage.group_storage.content_, alloc);
}

void group_field_instruction::destruct_value(value_storage_t& storage,
                                             allocator*       alloc) const
{
  if (storage.group_storage.content_) {
    destruct_group_subfields(storage.group_storage.content_, alloc);
    alloc->deallocate(storage.group_storage.content_);
  }
}

void group_field_instruction::copy_value(const value_storage_t& src,
                                         value_storage_t&       dest,
                                         allocator*             alloc) const
{
  dest.group_storage.present_ = src.group_storage.present_;
  dest.group_storage.content_ =
    static_cast<value_storage_t*>(alloc->allocate( this->group_content_byte_count() ));

  copy_group_subfields(src.group_storage.content_, dest.group_storage.content_, alloc);
}

void group_field_instruction::accept(field_instruction_visitor& visitor,
                                     void*                      context) const
{
  visitor.visit(this, context);
}

void group_field_instruction::ensure_valid_storage(value_storage_t& storage,
                                                   allocator*       alloc) const
{
  if (storage.group_storage.content_ == 0) {
    // group field is never used for a dictionary key; so, we won't use this
    // function for reseting a key and thus no memory deallocation is required.
    storage.group_storage.content_ =
      static_cast<value_storage_t*>(alloc->allocate( this->group_content_byte_count() ));
    memset(storage.group_storage.content_, 0, this->group_content_byte_count());
  }
}

/////////////////////////////////////////////////////////

void sequence_field_instruction::construct_sequence_elements(value_storage_t& storage,
                                                             std::size_t      start,
                                                             std::size_t      length,
                                                             allocator*       alloc) const
{
  value_storage_t* elements = static_cast<value_storage_t*>(storage.array_storage.content_);
  for (std::size_t i = start; i < start+length; ++i ) {
    construct_group_subfields(&elements[i*subinstructions_count_], alloc);
  }
}

void sequence_field_instruction::destruct_sequence_elements(value_storage_t& storage,
                                                            std::size_t      start,
                                                            std::size_t      length,
                                                            allocator*       alloc) const
{
  value_storage_t* elements = static_cast<value_storage_t*>(storage.array_storage.content_);
  for (std::size_t i = start; i < start+length; ++i ) {
    destruct_group_subfields(&elements[i*subinstructions_count_], alloc);
  }
}

void sequence_field_instruction::construct_value(value_storage_t& storage,
                                                 allocator*       alloc ) const
{
  // len_ == 0 is reserve for null/absent
  storage.array_storage.len_ = optional() ? 0 : sequence_length_instruction_->initial_value()+1;
  if (sequence_length_instruction_ && sequence_length_instruction_->initial_value() > 0) {
    std::size_t element_size = this->group_content_byte_count();
    std::size_t reserve_size = (sequence_length_instruction_->initial_value())*element_size;
    storage.array_storage.content_ = 0;
    storage.array_storage.capacity_ =  alloc->reallocate(storage.array_storage.content_, 0, reserve_size)/element_size;
    construct_sequence_elements(storage,0, sequence_length_instruction_->initial_value(), alloc);
  }
  else {
    storage.array_storage.content_ = 0;
    storage.array_storage.capacity_ = 0;
  }
}

void sequence_field_instruction::destruct_value(value_storage_t& storage,
                                                allocator*       alloc ) const
{
  if (storage.array_storage.capacity_ && storage.array_length() > 0) {
    destruct_sequence_elements(storage, 0, storage.array_length(), alloc);
    alloc->deallocate(storage.array_storage.content_);
  }
}

void sequence_field_instruction::copy_value(const value_storage_t& src,
                                            value_storage_t&       dest,
                                            allocator*             alloc) const
{
  std::size_t size = dest.array_storage.len_;

  if (size > 0)
    --size;

  dest.array_storage.len_ = src.array_storage.len_;

  if (size) {
    std::size_t element_size = this->group_content_byte_count();
    std::size_t reserve_size = size*element_size;

    dest.array_storage.content_ = 0;
    dest.array_storage.capacity_ =  alloc->reallocate(dest.array_storage.content_, 0, reserve_size)/element_size;

    const value_storage_t* src_elements =
      *static_cast<const value_storage_t**>(src.array_storage.content_);
    value_storage_t* dest_elements = *static_cast<value_storage_t**>(dest.array_storage.content_);

    for (std::size_t i = 0; i < size; ++i ) {
      copy_group_subfields(&src_elements[i], &dest_elements[i], alloc);
    }
  }
  else {
    dest.array_storage.content_ = 0;
    dest.array_storage.capacity_ = 0;
  }
}

void sequence_field_instruction::accept(field_instruction_visitor& visitor,
                                        void*                      context) const
{
  visitor.visit(this, context);
}

/////////////////////////////////////////////////////////

void template_instruction::construct_value(value_storage_t& storage,
                                           value_storage_t* fields_storage,
                                           allocator*       alloc,
                                           bool             construct_subfields) const
{
  if (fields_storage) {
    storage.group_storage.own_content_ = false;
  }
  else {
    storage.group_storage.own_content_ = true;
    fields_storage = static_cast<value_storage_t*>(
      alloc->allocate(this->group_content_byte_count()));
  }
  storage.group_storage.content_ = fields_storage;

  if (construct_subfields)
    construct_group_subfields(fields_storage, alloc);
  else
    memset(fields_storage, 0, this->group_content_byte_count());
}

void template_instruction::copy_construct_value(value_storage_t&       storage,
                                                value_storage_t*       fields_storage,
                                                allocator*             alloc,
                                                const value_storage_t* src) const
{
  if (fields_storage) {
    storage.group_storage.own_content_ = false;
  }
  else {
    storage.group_storage.own_content_ = true;
    fields_storage = static_cast<value_storage_t*>(
      alloc->allocate(this->group_content_byte_count()));
  }
  storage.group_storage.content_ = fields_storage;
  copy_group_subfields(src->group_storage.content_,
                       fields_storage,
                       alloc);

}

void template_instruction::accept(field_instruction_visitor& visitor,
                                  void*                      context) const
{
  visitor.visit(this, context);
}

void templateref_instruction::construct_value(value_storage_t& storage,
                                              allocator*       alloc) const
{
  storage.templateref_storage.instruction_storage.instruction_ = 0;
  storage.group_storage.content_ = 0;
}

void templateref_instruction::destruct_value(value_storage_t& storage,
                                             allocator*       alloc) const
{
  if (storage.templateref_storage.instruction_storage.instruction_ && storage.templateref_storage.content_) {
    storage.templateref_storage.instruction_storage.instruction_->destruct_group_subfields(
      static_cast<value_storage_t*>(storage.templateref_storage.content_),
      alloc);
    alloc->deallocate(storage.group_storage.content_);
  }
}

void templateref_instruction::copy_value(const value_storage_t& src,
                                         value_storage_t&       dest,
                                         allocator*             alloc) const
{
  dest.templateref_storage.instruction_storage.instruction_ = src.templateref_storage.instruction_storage.instruction_;
  if (src.templateref_storage.instruction_storage.instruction_) {
    dest.templateref_storage.content_ =
      static_cast<value_storage_t*>(alloc->allocate( dest.templateref_storage.instruction_storage.instruction_->group_content_byte_count() ));

    dest.templateref_storage.instruction_storage.instruction_->copy_group_subfields(
      src.templateref_storage.content_,
      dest.templateref_storage.content_,
      alloc);
  }
}

void templateref_instruction::accept(field_instruction_visitor& visitor, void* context) const
{
  visitor.visit(this, context);
}

templateref_instruction* templateref_instruction::instance()
{
  static templateref_instruction inst("","");
  return &inst;
}

}
