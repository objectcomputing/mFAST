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
#pragma once

#include "catch.hpp"
#include <mfast/malloc_allocator.h>
#include <set>
#include <cstring>

class debug_allocator
  : public mfast::malloc_allocator
{
  public:

    debug_allocator()
    {
    }

    ~debug_allocator()
    {
      CHECK(leased_addresses_.size() == 0U);
    }

    virtual void* allocate(std::size_t s) override
    {
      void* pointer = std::malloc(s);
      if (pointer == nullptr) throw std::bad_alloc();
      leased_addresses_.insert(pointer);
      return pointer;
    }

    virtual std::size_t reallocate(void*& pointer, std::size_t old_size, std::size_t new_size) override
    {
      new_size = std::max<std::size_t>(2*new_size, 64) & (~63);

      void* old_ptr = pointer;
      if (pointer) {
        pointer = std::realloc(pointer, new_size);
        std::memset(static_cast<char*>(pointer)+old_size, 0xFF, new_size-old_size);
      }
      else
        pointer = std::malloc(new_size);
      leased_addresses_.erase(old_ptr);
      if (pointer == nullptr) {
        std::free(old_ptr);
        throw std::bad_alloc();
      }
      leased_addresses_.insert(pointer);
      return new_size;
    }

    virtual void deallocate(void* pointer, std::size_t) override
    {

      CHECK(leased_addresses_.count(pointer) !=0 );
      std::free(pointer);
      leased_addresses_.erase(pointer);
    }

  protected:
    std::set<void*> leased_addresses_;
};

#ifndef UINT64_MAX
#define UINT64_MAX (18446744073709551615ULL)
#endif

#ifndef INT64_MAX
#define INT64_MAX (9223372036854775807LL)
#endif

#ifndef INT32_MIN
#define INT32_MIN (-2147483647L-1)
#endif


