// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.
#pragma once

#include <cstddef>
#include <algorithm>
#include "mfast/mfast_export.h"
namespace mfast {
/// An abstract interface for allocators.
///
class MFAST_EXPORT allocator {
public:
  /// Allocate exactly n bytes for the memory that is not subjected to regrow
  ///
  /// @param n The amount of memory in bytes to be allocated.
  /// @throws std::bad_alloc on failure.
  virtual void *allocate(std::size_t n) = 0;

  /// Reallocate at least @a new_size bytes of memory pointed by @a pointer.
  ///
  /// @param [in, out] pointer The memory address to be reallocated. If \a
  /// pointer is not 0, it
  /// must be one
  ///                of the addresses previously returned by reallocate().
  /// @param old_size The amount of memory previously allocated for the memory
  /// starting at \a
  /// pointer. Must be the one
  ///                 previously returned by reallocate() or 0 if \a pointer is
  ///                 also 0.
  /// @param new_size The amount of memory in bytes requested.
  /// @return The size of memory in bytes that is available for user. Notice the
  /// return value will
  /// always greater
  /// or equal to \a new_size; otherwise an exception is thrown.
  /// @throws std::bad_alloc on failure.
  virtual std::size_t reallocate(void *&pointer, std::size_t old_size,
                                 std::size_t new_size) = 0;

  /// Deallocate a memory of size \a n
  ///
  /// Notice that \a pointer must be the one returned by  allocate() or
  /// rellocate() and \a n
  /// must be the one specified in allocate() or the returned value of
  /// reallocate().
  virtual void deallocate(void *pointer, std::size_t n) = 0;

  /// Returns all the allocated memory blocks back to this allocator in one shot
  /// without calling
  /// deallocate() for each prevously allocated memory blocks from this
  /// allocator individually.
  ///
  /// This function returns false by default. Meaning that the allocator does
  /// not support reset()
  /// operation.
  ///
  /// @return true if successful.
  // virtual bool reset();
};
}
