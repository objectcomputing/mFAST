// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.
#include <cstring>
#include <algorithm>
#include "fast_ostreambuf.h"

namespace mfast {

void fast_ostreambuf::overflow(std::size_t) { throw buffer_overflow_error(); }

std::size_t fast_ostreambuf::length() const { return pptr_ - pbase_; }

void fast_ostreambuf::write_bytes_at(const char *data, std::size_t n,
                                     std::size_t offset, bool shrink) {
  assert((pbase_ + offset + n) <= pptr_);
  std::copy(data, data + n, pbase_ + offset);

  if (shrink) {
    std::size_t index = offset + n - 1;
    if (pbase_[index] == '\x80') {
      --index;
      for (; index > 0 && pbase_[index] == 0; --index)
        ;

      if (pbase_[index] & '\x80') {
        ++index;
      }
      pbase_[index] |= '\x80';
      ++index;
      const char *src = pbase_ + offset + n;
      std::memmove(pbase_ + index, src, pptr_ - src);

      pptr_ -= (offset + n - index);
    }
  }
}
}
