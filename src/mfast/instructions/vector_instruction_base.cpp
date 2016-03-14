// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.

#include "vector_instruction_base.h"

namespace mfast {
void vector_field_instruction_base::construct_value(value_storage &storage,
                                                    allocator *) const {
  storage = value_storage();
  storage.of_array.defined_bit_ = 1;
}

void vector_field_instruction_base::destruct_value(value_storage &storage,
                                                   allocator *alloc) const {
  if (storage.of_array.capacity_in_bytes_) {
    alloc->deallocate(storage.of_array.content_,
                      storage.of_array.capacity_in_bytes_);
  }
}

void vector_field_instruction_base::copy_construct_value(
    const value_storage &src, value_storage &dest, allocator *alloc,
    value_storage *) const {
  dest.of_array.defined_bit_ = 1;
  dest.of_array.len_ = src.of_array.len_;
  if (src.of_array.len_) {
    dest.of_array.content_ = nullptr;
    dest.of_array.capacity_in_bytes_ = alloc->reallocate(
        dest.of_array.content_, 0, src.of_array.len_ * element_size_);
    std::memcpy(dest.of_array.content_, src.of_array.content_,
                src.of_array.len_ * element_size_);
  } else {
    dest.of_array.capacity_in_bytes_ = 0;
    dest.of_array.content_ = nullptr;
  }
}

} /* mfast */
