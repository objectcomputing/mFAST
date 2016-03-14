// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.

#include "group_instruction.h"

namespace mfast {
void group_field_instruction::construct_group_subfields(
    value_storage *subfields, allocator *alloc) const {
  for (uint32_t i = 0; i < this->subinstructions_.size(); ++i) {
    this->subinstructions_[i]->construct_value(subfields[i], alloc);
  }
}

void group_field_instruction::destruct_group_subfields(value_storage *subfields,
                                                       allocator *alloc) const {
  for (uint32_t i = 0; i < this->subinstructions_.size(); ++i) {
    this->subinstructions_[i]->destruct_value(subfields[i], alloc);
  }
}

int group_field_instruction::find_subinstruction_index_by_id(
    uint32_t id) const {
  for (uint32_t i = 0; i < this->subinstructions_.size(); ++i) {
    if (this->subinstructions_[i]->id() == id)
      return i;
  }
  return -1;
}

int group_field_instruction::find_subinstruction_index_by_name(
    const char *name) const {
  for (uint32_t i = 0; i < this->subinstructions_.size(); ++i) {
    if (std::strcmp(this->subinstructions_[i]->name(), name) == 0)
      return i;
  }
  return -1;
}

// deep copy
void group_field_instruction::copy_group_subfields(
    const value_storage *src_subfields, value_storage *dest_subfields,
    allocator *alloc) const {
  for (uint32_t i = 0; i < this->subinstructions_.size(); ++i) {
    this->subinstructions_[i]->copy_construct_value(src_subfields[i],
                                                    dest_subfields[i], alloc);
  }
}

void group_field_instruction::construct_value(value_storage &storage,
                                              allocator *alloc) const {
  storage.of_group.present_ = !optional();
  storage.of_group.is_link_ = 0;
  // group field is never used for a dictionary key; so, we won't use this
  // function for reseting a key and thus no memory deallocation is required.
  storage.of_group.content_ =
      static_cast<value_storage *>(alloc->allocate(group_content_byte_count()));
  storage.of_group.own_content_ = true;
  construct_group_subfields(storage.of_group.content_, alloc);
}

void group_field_instruction::destruct_value(value_storage &storage,
                                             allocator *alloc) const {
  if (storage.of_group.content_) {
    if (!storage.of_group.is_link_)
      destruct_group_subfields(storage.of_group.content_, alloc);
    if (storage.of_group.own_content_) {
      alloc->deallocate(storage.of_group.content_, group_content_byte_count());
    }
  }
}

void group_field_instruction::construct_value(value_storage &storage,
                                              value_storage *fields_storage,
                                              allocator *alloc,
                                              bool construct_subfields) const {
  storage.of_group.own_content_ = fields_storage == nullptr;
  storage.of_group.is_link_ = !construct_subfields;
  storage.of_group.content_ =
      fields_storage ? fields_storage
                     : static_cast<value_storage *>(
                           alloc->allocate(this->group_content_byte_count()));

  if (construct_subfields)
    construct_group_subfields(storage.of_group.content_, alloc);
  else
    memset(storage.of_group.content_, 0, this->group_content_byte_count());
}

void group_field_instruction::copy_construct_value(
    const value_storage &src, value_storage &dest, allocator *alloc,
    value_storage *dest_fields_storage) const {
  dest.of_group.present_ = src.of_group.present_;
  dest.of_group.own_content_ = dest_fields_storage == nullptr;
  dest.of_group.is_link_ = 0;
  dest.of_group.content_ =
      dest_fields_storage ? dest_fields_storage
                          : static_cast<value_storage *>(
                                alloc->allocate(group_content_byte_count()));

  copy_group_subfields(src.of_group.content_, dest.of_group.content_, alloc);
}

group_field_instruction *
group_field_instruction::clone(arena_allocator &alloc) const {
  return new (alloc) group_field_instruction(*this);
}

void group_field_instruction::link_value(value_storage &storage,
                                         value_storage *fields_storage,
                                         allocator *alloc) const {
  this->destruct_value(storage, alloc);
  storage.of_group.present_ = 1;
  storage.of_group.own_content_ = 0;
  storage.of_group.is_link_ = 1;
  storage.of_group.content_ = fields_storage;
}

void group_field_instruction::set_subinstructions(
    instructions_view_t instructions) {
  subinstructions_ = instructions;
  segment_pmap_size_ = 0;
  for (const field_instruction *inst : instructions) {
    segment_pmap_size_ += inst->pmap_size();
  }
}

} /* mfast */
