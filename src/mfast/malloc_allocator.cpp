// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.
#include "malloc_allocator.h"

namespace mfast {

malloc_allocator *malloc_allocator::instance() {
  static malloc_allocator alloc;
  return &alloc;
}

void *malloc_allocator::allocate(std::size_t s) {
  void *pointer = std::malloc(s);
  if (pointer == nullptr)
    throw std::bad_alloc();
  return pointer;
}

std::size_t malloc_allocator::reallocate(void *&pointer,
                                         std::size_t /* old_size */,
                                         std::size_t new_size) {
  // make the new_size at least 64 bytes
  new_size = std::max<std::size_t>(2 * new_size, 64) & (~63);
  void *old_ptr = pointer;
  pointer = std::realloc(pointer, new_size);
  if (pointer == nullptr) {
    std::free(old_ptr);
    throw std::bad_alloc();
  }
  return new_size;
}

void malloc_allocator::deallocate(void *pointer, std::size_t) {
  std::free(pointer);
}
}
