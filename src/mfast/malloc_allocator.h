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
#ifndef MALLOC_ALLOCATOR_H_M03J5JS6
#define MALLOC_ALLOCATOR_H_M03J5JS6
#include "allocator.h"
#include <cstdlib>
#include <new>

namespace mfast {
  class MFAST_EXPORT malloc_allocator
    : public allocator
  {
  public:
    static malloc_allocator* instance();

    virtual void* allocate(std::size_t s);
    virtual std::size_t reallocate(void*& pointer, std::size_t old_size, std::size_t new_size);
    virtual void deallocate(void* pointer,std::size_t);
  };

}

#endif /* end of include guard: MALLOC_ALLOCATOR_H_M03J5JS6 */
