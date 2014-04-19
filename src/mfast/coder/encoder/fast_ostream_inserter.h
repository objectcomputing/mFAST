#ifndef FAST_OSTREAM_INSERTER_H_WQ683ZCZ
#define FAST_OSTREAM_INSERTER_H_WQ683ZCZ

#include "mfast/int_ref.h"
#include "mfast/string_ref.h"
#include "mfast/decimal_ref.h"
#include "fast_ostream.h"

namespace mfast
{
  template <typename U>
  inline fast_ostream& operator << (fast_ostream& strm, const int_cref<U>& cref)
  {
    strm.encode(cref.value(), cref.instruction()->is_nullable(), cref.absent());
    return strm;
  }

  inline fast_ostream& operator << (fast_ostream& strm, const exponent_cref& cref)
  {
    strm.encode(cref.value(), cref.instruction()->is_nullable(), cref.absent());
    return strm;
  }

  inline fast_ostream& operator << (fast_ostream& strm, const ascii_string_cref& cref)
  {
    strm.encode(cref.c_str(),static_cast<uint32_t>( cref.size()), cref.instruction()->is_nullable(), cref.instruction());
    return strm;
  }

  inline fast_ostream& operator << (fast_ostream& strm, const unicode_string_cref& cref)
  {
    strm.encode(cref.c_str(), static_cast<uint32_t>(cref.size()), cref.instruction()->is_nullable(), cref.instruction());
    return strm;
  }

  inline fast_ostream& operator << (fast_ostream& strm, const byte_vector_cref& cref)
  {
    strm.encode(cref.begin(), static_cast<uint32_t>(cref.size()), cref.instruction()->is_nullable(), cref.instruction());
    return strm;
  }

  inline fast_ostream& operator << (fast_ostream& strm, const decimal_cref& cref)
  {
    strm.encode(cref.exponent(), cref.instruction()->is_nullable(), cref.absent());
    if (cref.present()) {
      strm.encode(cref.mantissa(), false, false);
    }
    return strm;
  }

}

#endif /* end of include guard: FAST_OSTREAM_INSERTER_H_WQ683ZCZ */
