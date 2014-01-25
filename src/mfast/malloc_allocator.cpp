// Copyright (c) 2013, 2014, Huang-Ming Huang,  Object Computing, Inc.
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
#include "malloc_allocator.h"


namespace mfast {

  malloc_allocator*
  malloc_allocator::instance()
  {
    static malloc_allocator alloc;
    return &alloc;
  }

  void*
  malloc_allocator::allocate(std::size_t s)
  {
    void* pointer = std::malloc(s);
    if (pointer == 0) throw std::bad_alloc();
    return pointer;
  }

  std::size_t
  malloc_allocator::reallocate(void*&      pointer,
                               std::size_t /* old_size */,
                               std::size_t new_size)
  {
    // make the new_size at least 64 bytes
    new_size = std::max<std::size_t>(2*new_size, 64) & (~63);
    void* old_ptr = pointer;
    pointer = std::realloc(pointer, new_size);
    if (pointer == 0) {
      std::free(old_ptr);
      throw std::bad_alloc();
    }
    return new_size;
  }

  void
  malloc_allocator::deallocate(void* pointer, std::size_t)
  {
    std::free(pointer);
  }

}
