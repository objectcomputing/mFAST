// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.
#pragma once

#include <stdint.h>
#include "fast_istreambuf.h"

#ifndef __GNUC__
#define __builtin_expect(expr, expected_value) (expr)
#endif

namespace mfast {
class decoder_presence_map {
public:
  decoder_presence_map() : cur_bitmap_(0), mask_(0), continue_(nullptr) {}
  bool is_next_bit_set() {
    mask_ >>= 1;
    if (__builtin_expect(mask_ == 0 && continue_, 0)) {
      load(continue_);
      mask_ >>= 1;
    }
    bool result = (cur_bitmap_ & mask_) != 0;
    return result;
  }

  bool load(fast_istreambuf &buf) {
    const char *addr = buf.gptr();
    bool result = load(addr);
    buf.gbump(addr - buf.gptr());
    return result;
  }

  // only used for test case verification
  size_t mask() const { return mask_; }

private:
  bool load(const char *&addr) {
    const int max_load_byes = sizeof(size_t) * 8 / 7;
    mask_ = 1;
    for (int i = 0; i < max_load_byes; ++i, ++addr) {
      char c = *addr;
      cur_bitmap_ <<= 7;
      cur_bitmap_ |= c & '\x7F';
      mask_ <<= 7;
      if ('\x80' == (c & '\x80')) {
        ++addr;
        continue_ = nullptr;
        return true;
      }
    }
    continue_ = addr;
    return false;
  }

  size_t cur_bitmap_;
  size_t mask_;
  const char *continue_;
  friend std::ostream &operator<<(std::ostream &,
                                  const mfast::decoder_presence_map &);
};
}
