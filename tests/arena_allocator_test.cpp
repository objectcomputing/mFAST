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
#include "catch.hpp"

#include <mfast/arena_allocator.h>
#include <cstring>
#include <algorithm>

using namespace mfast;



TEST_CASE("test arena_allocatore", "[arena_allocatore_test]")
{
  arena_allocator alloc;

  void* block1 = alloc.allocate(arena_allocator::default_chunk_size/2);
  alloc.reset();
  void* block2 = alloc.allocate(arena_allocator::default_chunk_size/2);

  REQUIRE(block1 ==  block2);
  // use all the remaing space of current chunk
  //void* block3 =
  alloc.allocate(arena_allocator::chunk_user_size - arena_allocator::default_chunk_size/2);

  // now we will get memory from another chunk
  void* block4 = alloc.allocate(arena_allocator::default_chunk_size/2);
  alloc.reset();

  void* block5 = alloc.allocate(arena_allocator::chunk_user_size);
  REQUIRE(block4 ==  block5);

  // this should get the memory from the first chunk
  void* block6 = alloc.allocate(arena_allocator::chunk_user_size);
  REQUIRE(block6 ==  block1);

  // make sure we can allocate memory far larger than the default chunk size
  char* block7 = static_cast<char*>(alloc.allocate(3*arena_allocator::default_chunk_size));

  // if the returned memroy block is smaller than need, we should get a memory access error at this point.
  memset(block7, 0, 3*arena_allocator::default_chunk_size);
}

