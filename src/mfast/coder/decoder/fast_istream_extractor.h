// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.
#pragma once

#include "../../int_ref.h"
#include "../../string_ref.h"
#include "../../decimal_ref.h"
#include "fast_istream.h"

namespace mfast {
template <typename U>
inline fast_istream &operator>>(fast_istream &strm, const int_mref<U> &mref) {
  U value;
  if (!strm.decode(value, mref.instruction()->is_nullable()))
    mref.omit();
  else
    mref.as(value);
  return strm;
}

inline fast_istream &operator>>(fast_istream &strm, const exponent_mref &mref) {
  int16_t value;
  if (!strm.decode(value, mref.instruction()->is_nullable()))
    mref.omit();
  else
    mref.as(value);
  return strm;
}

inline fast_istream &operator>>(fast_istream &strm,
                                const ascii_string_mref &mref) {
  const char *buf;
  uint32_t len;
  if (strm.decode(buf, len, mref.instruction(),
                  mref.instruction()->is_nullable())) {
    mref.assign(buf, buf + len);
    if (len > 0)
      mref[len - 1] &= '\x7F';
  } else {
    mref.omit();
  }
  return strm;
}

inline fast_istream &operator>>(fast_istream &strm,
                                const unicode_string_mref &mref) {
  const char *buf;
  uint32_t len;
  if (strm.decode(buf, len, mref.instruction(),
                  mref.instruction()->is_nullable())) {
    mref.assign(buf, buf + len);
  } else {
    mref.omit();
  }
  return strm;
}

inline fast_istream &operator>>(fast_istream &strm,
                                const byte_vector_mref &mref) {
  const unsigned char *buf;
  uint32_t len;
  if (strm.decode(buf, len, mref.instruction(),
                  mref.instruction()->is_nullable())) {
    mref.assign(buf, buf + len);
  } else {
    mref.omit();
  }
  return strm;
}

inline fast_istream &operator>>(fast_istream &strm, const decimal_mref &mref) {
  value_storage *storage = mref.storage();
  if (strm.decode(storage->of_decimal.exponent_,
                  mref.instruction()->is_nullable())) {
    storage->present(true);
    strm.decode(storage->of_decimal.mantissa_, false);
  } else {
    mref.omit();
  }
  return strm;
}
}
