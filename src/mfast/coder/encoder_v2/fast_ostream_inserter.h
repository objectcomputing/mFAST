#pragma once

#include "../../int_ref.h"
#include "../../string_ref.h"
#include "../../decimal_ref.h"
#include "../../ext_ref.h"
#include "../encoder/fast_ostream.h"

namespace mfast {
namespace coder {
template <typename T>
inline fast_ostream &operator<<(fast_ostream &strm, const T &ext_ref) {
  typename T::cref_type cref = ext_ref.get();
  strm.encode(cref.value(), cref.absent(), ext_ref.nullable());
  return strm;
}

template <typename Operator, typename Properties>
inline fast_ostream &
operator<<(fast_ostream &strm,
           const ext_cref<ascii_string_cref, Operator, Properties> &ext_ref) {
  ascii_string_cref cref = ext_ref.get();
  strm.encode(cref.c_str(), static_cast<uint32_t>(cref.size()),
              cref.instruction(), ext_ref.nullable());
  return strm;
}

template <typename Operator, typename Properties>
inline fast_ostream &
operator<<(fast_ostream &strm,
           const ext_cref<unicode_string_cref, Operator, Properties> &ext_ref) {
  unicode_string_cref cref = ext_ref.get();
  strm.encode(cref.c_str(), static_cast<uint32_t>(cref.size()),
              cref.instruction(), ext_ref.nullable());
  return strm;
}

template <typename Operator, typename Properties>
inline fast_ostream &
operator<<(fast_ostream &strm,
           const ext_cref<byte_vector_cref, Operator, Properties> &ext_ref) {
  byte_vector_cref cref = ext_ref.get();
  strm.encode(cref.begin(), static_cast<uint32_t>(cref.size()),
              cref.instruction(), ext_ref.nullable());
  return strm;
}

template <typename Operator, typename Properties>
inline fast_ostream &
operator<<(fast_ostream &strm,
           const ext_cref<decimal_cref, Operator, Properties> &ext_ref) {
  decimal_cref cref = ext_ref.get();
  strm.encode(cref.exponent(), cref.absent(), ext_ref.nullable());
  if (cref.present()) {
    strm.encode(cref.mantissa(), false, false_type());
  }
  return strm;
}

} /* coder */

} /* mfast */
