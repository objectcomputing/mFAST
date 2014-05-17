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
#ifndef FAST_ISTREAM_EXTRACTOR_H_AMFDBCO
#define FAST_ISTREAM_EXTRACTOR_H_AMFDBCO

#include "mfast/int_ref.h"
#include "mfast/string_ref.h"
#include "mfast/decimal_ref.h"
#include "fast_istream.h"

namespace mfast
{

  template <typename U>
  inline fast_istream& operator >> (fast_istream& strm, const int_mref<U>& mref)
  {
    U value;
    if (!strm.decode(value, mref.instruction()->is_nullable()))
      mref.omit();
    else
      mref.as(value);
    return strm;
  }

  inline fast_istream& operator >> (fast_istream& strm, const exponent_mref& mref)
  {
    int16_t value;
    if (!strm.decode(value, mref.instruction()->is_nullable()))
      mref.omit();
    else
      mref.as(value);
    return strm;
  }

  inline fast_istream& operator >> (fast_istream& strm, const ascii_string_mref& mref)
  {
    const char* buf;
    uint32_t len;
    if (strm.decode(buf, len, mref.instruction()->is_nullable(), mref.instruction())) {
      mref.assign(buf, buf+len);
      if (len > 0)
        mref[len-1] &= '\x7F';
    }
    else {
      mref.omit();
    }
    return strm;
  }

  inline fast_istream& operator >> (fast_istream& strm, const unicode_string_mref& mref)
  {
    const char* buf;
    uint32_t len;
    if (strm.decode(buf, len, mref.instruction()->is_nullable(), mref.instruction()))
    {
      mref.assign(buf, buf+len);
    }
    else {
      mref.omit();
    }
    return strm;
  }

  inline fast_istream& operator >> (fast_istream& strm, const byte_vector_mref& mref)
  {
    const unsigned char* buf;
    uint32_t len;
    if (strm.decode(buf, len, mref.instruction()->is_nullable(), 0))
    {
      mref.assign(buf, buf+len);
    }
    else {
      mref.omit();
    }
    return strm;
  }

  inline fast_istream& operator >> (fast_istream& strm, const decimal_mref& mref)
  {
    value_storage* storage = mref.storage();
    if (strm.decode(storage->of_decimal.exponent_,
                    mref.instruction()->is_nullable()))
    {
      storage->present(true);
      strm.decode(storage->of_decimal.mantissa_, false);
    }
    else {
      mref.omit();
    }
    return strm;
  }

}

#endif /* end of include guard: FAST_ISTREAM_EXTRACTOR_H_AMFDBCO */
