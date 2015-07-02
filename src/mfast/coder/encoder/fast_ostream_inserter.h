#pragma once

#include "mfast/int_ref.h"
#include "mfast/string_ref.h"
#include "mfast/decimal_ref.h"
#include "fast_ostream.h"

namespace mfast {
template <typename U>
inline fast_ostream &operator<<(fast_ostream &strm, const int_cref<U> &cref) {
  strm.encode(cref.value(), cref.absent(), cref.instruction()->is_nullable());
  return strm;
}

inline fast_ostream &operator<<(fast_ostream &strm, const exponent_cref &cref) {
  strm.encode(cref.value(), cref.absent(), cref.instruction()->is_nullable());
  return strm;
}

inline fast_ostream &operator<<(fast_ostream &strm,
                                const ascii_string_cref &cref) {
  strm.encode(cref.c_str(), static_cast<uint32_t>(cref.size()),
              cref.instruction(), cref.instruction()->is_nullable());
  return strm;
}

inline fast_ostream &operator<<(fast_ostream &strm,
                                const unicode_string_cref &cref) {
  strm.encode(cref.c_str(), static_cast<uint32_t>(cref.size()),
              cref.instruction(), cref.instruction()->is_nullable());
  return strm;
}

inline fast_ostream &operator<<(fast_ostream &strm,
                                const byte_vector_cref &cref) {
  strm.encode(cref.begin(), static_cast<uint32_t>(cref.size()),
              cref.instruction(), cref.instruction()->is_nullable());
  return strm;
}

inline fast_ostream &operator<<(fast_ostream &strm, const decimal_cref &cref) {
  strm.encode(cref.exponent(), cref.absent(),
              cref.instruction()->is_nullable());
  if (cref.present()) {
    strm.encode(cref.mantissa(), false, false);
  }
  return strm;
}
}
