// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.
#pragma once

#include "../../int_ref.h"
#include "../../string_ref.h"
#include "../../decimal_ref.h"
#include "../../ext_ref.h"
#include "../decoder/fast_istream.h"

namespace mfast {
namespace coder {
template <typename T>
inline fast_istream &operator>>(fast_istream &strm, const T &ext_ref) {
  typename T::mref_type::value_type value;
  if (!strm.decode(value, ext_ref.nullable()))
    ext_ref.omit();
  else
    ext_ref.set().as(value);
  return strm;
}

template <typename Operator, typename Properties>
inline fast_istream &
operator>>(fast_istream &strm,
           const ext_mref<ascii_string_mref, Operator, Properties> &ext_ref) {
  const char *buf;
  ascii_string_mref mref = ext_ref.set();
  uint32_t len;
  if (strm.decode(buf, len, mref.instruction(), ext_ref.nullable())) {
    mref.assign(buf, buf + len);
    if (len > 0)
      mref[len - 1] &= '\x7F';
  } else {
    mref.omit();
  }
  return strm;
}

template <typename Operator, typename Properties>
inline fast_istream &
operator>>(fast_istream &strm,
           const ext_mref<unicode_string_mref, Operator, Properties> &ext_ref) {
  const char *buf;
  uint32_t len;
  unicode_string_mref mref = ext_ref.set();
  if (strm.decode(buf, len, mref.instruction(), ext_ref.nullable())) {
    mref.assign(buf, buf + len);
  } else {
    mref.omit();
  }
  return strm;
}

template <typename Operator, typename Properties>
inline fast_istream &
operator>>(fast_istream &strm,
           const ext_mref<byte_vector_mref, Operator, Properties> &ext_ref) {
  const unsigned char *buf;
  uint32_t len;
  byte_vector_mref mref = ext_ref.set();
  if (strm.decode(buf, len, mref.instruction(), ext_ref.nullable())) {
    mref.assign(buf, buf + len);
  } else {
    mref.omit();
  }
  return strm;
}

template <typename Operator, typename Properties>
inline fast_istream &
operator>>(fast_istream &strm,
           const ext_mref<decimal_mref, Operator, Properties> &ext_ref) {
  decimal_mref mref = ext_ref.set();
  value_storage *storage = field_mref_core_access::storage_of(mref);
  if (strm.decode(storage->of_decimal.exponent_, ext_ref.nullable())) {
    storage->present(true);
    strm.decode(storage->of_decimal.mantissa_, false);
  } else {
    mref.omit();
  }
  return strm;
}

} /* coder */

} /* mfast */
