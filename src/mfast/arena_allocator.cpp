// Copyright (c) 2013, Huang-Ming Huang,  Object Computing, Inc.
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
#include "arena_allocator.h"
#include <algorithm>

namespace mfast {

void arena_allocator::free_list(memory_chunk* head)
{
  memory_chunk* tmp;
  while ( head ) {
    tmp = head->next_;
    free(head);
    head = tmp;
  };
}

arena_allocator::~arena_allocator()
{
  free_list(current_list_head_);
  free_list(free_list_head_);
}

void* arena_allocator::allocate(std::size_t n)
{
  // align n to the multiple of 8
  n = (n + 7) & ~7;

  if (current_list_head_->size() < n) {
    // current block does not have enough memory

    if (free_list_head_ && free_list_head_->size() >= n ) {
      // The head of free_list is big enough,
      // move the head of free_listto become the head of current_list
      memory_chunk* tmp = free_list_head_;
      free_list_head_ = free_list_head_->next_;

      tmp->next_ = current_list_head_;
      current_list_head_ = tmp;
    }
    else {
      if (current_list_head_->size()  >= 64) {
        // if current block is have plenty of free space, move it to the free_list
        memory_chunk* tmp_current_list_head_head = current_list_head_->next_;
        current_list_head_->next_ = free_list_head_;
        free_list_head_ = current_list_head_;
        current_list_head_ = tmp_current_list_head_head;
      }
      // allocate new memory chunk from the system
      std::size_t new_chunk_size = n+ sizeof(memory_chunk) - sizeof(uint64_t) // minimum size for the new block
                                   + (default_chunk_size -1) & ~(default_chunk_size -1); // make the size multiple of default_chunk_size

      void* block = malloc(new_chunk_size);
      current_list_head_ = new (block) memory_chunk(new_chunk_size, current_list_head_);
    }
  }
  char* result = current_list_head_->start_;
  current_list_head_->start_ += n;
  return result;
}

std::size_t
arena_allocator::reallocate(void*& pointer, std::size_t old_size, std::size_t new_size)
{
  // make the new_size at least 64 bytes
  new_size = std::max(2*new_size, 64UL) & (~63);
  void* old_pointer = pointer;
  pointer = this->allocate(new_size);
  std::memcpy(pointer, old_pointer, old_size);
  return new_size;
}


bool arena_allocator::reset()
{
  // first, find the tail of current_list
  memory_chunk* tmp = current_list_head_;
  while (tmp->next_ != 0) {
    tmp = tmp->next_;
    tmp->start_ = reinterpret_cast<char*>(tmp->user_memory);
  }
  // append the free_list_head_ to the tail of current_list
  tmp->next_ = free_list_head_;
  // make current_list_head_->next_ to become the new head of free_list
  free_list_head_ = current_list_head_->next_;

  // only keeps the head of current_list_head_ list, the reset of the current_list moves to the free_list
  current_list_head_->next_ = 0;
  current_list_head_->start_ = reinterpret_cast<char*>(current_list_head_->user_memory);
  return true;
}

void  arena_allocator::deallocate(void* pointer)
{
}
}
