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
#include "mfast/allocator.h"
#include <new>
#include <cstring>

namespace mfast {

  inline std::size_t align(std::size_t n, std::size_t x)
  {
    const std::size_t y = x-1;
    return (n + y) & ~y;
  }

  std::size_t
  allocator::reallocate(void*& pointer, std::size_t old_size, std::size_t new_size)
  {
    // make the new_size at least 64 bytes
    new_size = align(static_cast<std::size_t>(new_size), 64);
    void* old_pointer = pointer;
    pointer = this->allocate(new_size);
    std::memcpy(pointer, old_pointer, old_size);
    this->deallocate(old_pointer, old_size);
    return new_size;
  }

}
