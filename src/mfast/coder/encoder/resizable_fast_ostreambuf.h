// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.
#pragma once

#include "fast_ostreambuf.h"
#include <vector>

namespace mfast {
class resizable_fast_ostreambuf : public fast_ostreambuf {
public:
  resizable_fast_ostreambuf(std::vector<char> &buf)
      : fast_ostreambuf(nullptr, 0), buf_(buf) {
    std::size_t old_size = buf.size();
    std::size_t new_size = old_size + 1024;
    buf.resize(new_size);
    char *addr = &buf_[0];
    setp(addr, addr + old_size, addr + new_size);
  }

  virtual void overflow(std::size_t n) override {
    std::size_t len = length();
    buf_.resize(2 * (len + n));
    char *addr = &buf_[0];
    setp(addr, addr + len, addr + buf_.size());
  }

private:
  std::vector<char> &buf_;
};
}
