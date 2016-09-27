// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.
#pragma once

#include <stdexcept>

#include "../../exceptions.h"
#include <iostream>

namespace mfast {
class fast_istream;
class decoder_presence_map;

namespace coder {
struct fast_decoder_base;
}

class fast_istreambuf {
public:
  fast_istreambuf(const char *buf, std::size_t sz)
      : gptr_(buf), egptr_(buf + sz) {}
  size_t in_avail() const { return egptr_ - gptr_; }
  // get the length of the stop bit encoded entity
  std::size_t get_entity_length() {
    const std::size_t n = in_avail();
    for (std::size_t i = 0; i < n; ++i) {
      if (gptr_[i] & 0x80)
        return i + 1;
    }
    BOOST_THROW_EXCEPTION(fast_dynamic_error("Buffer underflow"));
  }

  const char *gptr() { return gptr_; }

protected:
  friend class fast_istream;
  friend class decoder_presence_map;
  friend class fast_decoder;
  friend struct coder::fast_decoder_base;

  void gbump(std::ptrdiff_t n) { gptr_ += n; }
  unsigned char sbumpc() {
    if (in_avail() < 1)
      BOOST_THROW_EXCEPTION(fast_dynamic_error("Buffer underflow"));
    return *(gptr_++);
  }

  char sgetc() const { return *gptr_; }
  const char *egptr() { return egptr_; }
  const char *gptr_, *egptr_;
};
}
