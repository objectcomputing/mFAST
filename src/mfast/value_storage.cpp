// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.
#include "value_storage.h"
#include <iostream>
#include <algorithm>
#include <cstdio>
#include <iterator>

namespace mfast {
std::istream &operator>>(std::istream &strm, decimal_value_storage &storage) {

  int64_t mantissa;
  int16_t exponent = 0;
  if (!(strm >> std::skipws >> mantissa)) {
    return strm;
  }
  // BOOST_THROW_EXCEPTION(json_decode_error(strm, "Expect decimal"));

  std::streambuf *sbuf = strm.rdbuf();

  int c = sbuf->sbumpc();
  if (c == '.') {
    bool negative = false;

    if (mantissa < 0) {
      negative = true;
      mantissa = -mantissa;
    }

    while ((c = sbuf->sbumpc()) != EOF && isdigit(c)) {
      mantissa *= 10;
      mantissa += c - '0';
      exponent -= 1;
    }

    if (negative) {
      mantissa = -mantissa;
    }
  }

  if (c == 'e' || c == 'E') {
    int16_t exp;
    if (!(strm >> exp)) {
      return strm;
    }
    exponent += exp;
  } else if (c != EOF) {
    strm.putback(c);
  }
  storage.mantissa(mantissa);
  storage.exponent(exponent);
  storage.storage_.present(1);
  return strm;
}

// Although C99 defined PRId64 macro in <inttypes.h> as the printf flag for
// int64_t,
// it's not available on MSVC. On the travis-ci platform, <inttypes.h> is
// available,
// but PRId64 is undefined. Using function overload is the only way I can make
// it
// portable without compiler warnining.
inline int snprint_int(char *buf, int buf_size, long x) {
#ifndef _MSC_VER
  return std::snprintf(buf, buf_size, "%ld", x);
#else
  return sprintf_s(buf, buf_size, "%ld", x);
#endif
}

inline int snprint_int(char *buf, int buf_size, long long x) {
#ifndef _MSC_VER
  return std::snprintf(buf, buf_size, "%lld", x);
#else
  return sprintf_s(buf, buf_size, "%lld", x);
#endif
}

std::ostream &operator<<(std::ostream &os,
                         const decimal_value_storage &storage) {
  int64_t mantissa = storage.mantissa();
  int exponent = storage.exponent();

  if (exponent >= 0) {
    os << mantissa;
    std::fill_n(std::ostream_iterator<char>(os), exponent, '0');
  } else if (exponent < 0) {
    char buf[128];

    int n = snprint_int(buf, 128, mantissa);

    char *p = buf;
    if (mantissa < 0) {
      os.put('-');
      --n;
      ++p;
    }
    if ((n + exponent) > 0) {
      os.write(p, n + exponent);
      os << '.' << p + n + exponent;
    } else {
      os << "0.";
      std::fill_n(std::ostream_iterator<char>(os), -exponent - n, '0');
      os << p;
    }
  }

  return os;
}

} /* mfast */
