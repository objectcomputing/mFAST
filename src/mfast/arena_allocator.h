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

#ifndef ARENA_ALLOCATOR_H_7IC1CLW6
#define ARENA_ALLOCATOR_H_7IC1CLW6

#include <new>
#include <stdint.h>
#include <cassert>
#include <cstddef>
#include "allocator.h"
namespace mfast {

  /// A special memory allocator where small memory blocks are allocated from a big memory chunk
  /// and those small memory blocks are all returned to the allocator in one shot.
  class MFAST_EXPORT arena_allocator
    : public allocator
  {
  public:
    arena_allocator();
    ~arena_allocator();

    virtual void* allocate(std::size_t n);
    virtual std::size_t reallocate(void*& pointer, std::size_t old_size, std::size_t new_size);

    /// Release all previously allocated memory blocks
    virtual bool reset();
    virtual void deallocate(void* pointer, std::size_t);

  private:

    struct memory_chunk_base {
      memory_chunk_base* next_;
      char* end_;
      char* start_;
      uint64_t user_memory[1];
    };

    // If we combine memory_chunk and memory_chunk_base, some older GCC would
    // complain when offsetof() is used because memory_chunk is not POD type
    // and it's impossible to disable the warning using pragma; therefore
    // I decide to separate them into two.å

    struct memory_chunk
      : memory_chunk_base
    {

      memory_chunk(std::size_t size, memory_chunk* next)
      {
        next_ = next;
        end_ = reinterpret_cast<char*>(this) + size;
        start_ = reinterpret_cast<char*>(user_memory);
        assert(size % sizeof(uint64_t) == 0);
      }

      std::size_t size() const
      {
        return end_ - start_;
      }

    };

    void free_list(memory_chunk_base* list);

    // We maintian two singlely linked list of memory chunks : current_list and free_list.
    // The head of current_list is where new smaller memory blocks are allocated from. The
    // reset of the current_list maintain the memory chunks which leased memory blocks are from.
    // The free_list maintain some free memory chunks that future memory block allocation can be
    // from if the available size of the head of current_list is not enough.
    memory_chunk* current_list_head_;
    memory_chunk* free_list_head_;

  public:
    enum {
      default_chunk_size=4096,
      chunk_user_size = default_chunk_size - offsetof(struct memory_chunk_base, user_memory)
    };

  };

}

inline void*
operator new (std::size_t size, mfast::arena_allocator & arena)
{
  return arena.allocate(size);
}

inline void*
operator new [] (std::size_t size, mfast::arena_allocator & arena)
{
  return arena.allocate(size);
}

inline void
operator delete (void* /* p */, mfast::arena_allocator & /* arena */)
{
}

inline void
operator delete[] (void* /* p */, mfast::arena_allocator & /* arena */)
{
}

#endif /* end of include guard: ARENA_ALLOCATOR_H_7IC1CLW6 */
