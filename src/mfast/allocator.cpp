// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.
#include "allocator.h"
#include <new>
#include <cstring>

namespace mfast {

inline std::size_t align(std::size_t n, std::size_t x) {
  const std::size_t y = x - 1;
  return (n + y) & ~y;
}

std::size_t allocator::reallocate(void *&pointer, std::size_t old_size,
                                  std::size_t new_size) {
  // make the new_size at least 64 bytes
  new_size = align(static_cast<std::size_t>(new_size), 64);
  void *old_pointer = pointer;
  pointer = this->allocate(new_size);
  std::memcpy(pointer, old_pointer, old_size);
  this->deallocate(old_pointer, old_size);
  return new_size;
}
}
