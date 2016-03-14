// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.
#pragma once
#include "allocator.h"
#include <cstdlib>
#include <new>

namespace mfast {
class MFAST_EXPORT malloc_allocator : public allocator {
public:
  static malloc_allocator *instance();

  virtual void *allocate(std::size_t s) override;
  virtual std::size_t reallocate(void *&pointer, std::size_t old_size,
                                 std::size_t new_size) override;
  virtual void deallocate(void *pointer, std::size_t) override;
};
}
