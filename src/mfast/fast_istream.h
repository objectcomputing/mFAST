// Copyright (c) 2013, Huang-Ming Huang,  Object Computing, Inc.
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
#ifndef FAST_ISTREAM_H_7X1JL4X6
#define FAST_ISTREAM_H_7X1JL4X6
#include <stdexcept>
#include <limits>
#include <boost/type_traits.hpp>

#include "decoder_presence_map.h"
#include "field_instruction.h"
#include "exceptions.h"
#include <iostream>

namespace mfast
{
class fast_istream;
std::ostream& operator << (std::ostream& os, const fast_istream& istream);
class fast_istream
{
  public:
    fast_istream(const char* buf, std::size_t sz)
      : gptr_(buf)
      , egptr_(buf+sz)
    {
#ifdef REPORT_OVERFLOW
      overflow_log_ = 0;
#endif
    }

    size_t in_avail() const
    {
      return egptr_-gptr_;
    }

    bool eof() const {
      return in_avail() == 0;
    }

    bool operator!() const {
       return in_avail() == 0;
    }

    // get the length of the stop bit encoded entity
    std::size_t
    get_entity_length()
    {
      const std::size_t n = in_avail();
      for (std::size_t i = 0; i < n; ++i)
      {
        if (gptr_[i] & 0x80)
          return i+1;
      }
      BOOST_THROW_EXCEPTION(fast_dynamic_error("Buffer underflow"));
    }


    /**
     * Decode an integer and store the result in @a result.
     *
     * @return false if the decoded value is null
     **/
    template <typename T>
    typename boost::enable_if< boost::is_integral<T> ,bool>::type
    decode(T& result, bool nullable);


    void decode(decoder_presence_map& pmap)
    {
      if (!pmap.load(gptr_)) {
        while (0 == (sbumpc() & 0x80)) ;
      }
    }

    /**
     * Decode an ascii string.
     *
     * Notice that this function only return the underlying buffer for a string.
     * The stop bit is still exists on the last bytes. In other words, if the
     * decoded string is "\x40\x41\x42", the returned ascii[2] will be "\xC2".
     * The caller needs to mask the stop bit.
     *
     * @param [out] ascii The decoded string
     * @param [out] len The decoded string length
     * @param [in] instruction The field instruction
     * @return false if the decoded value is null
     **/
    bool decode(const char*&                 ascii,
                uint32_t&                    len,
                bool                         nullable,
                const ascii_field_instruction* /* instruction */)
    {
      char c = *gptr_;
      ascii = gptr_;
      if ((c & '\x7F') == 0) {
        gptr_++;

        if (c == '\x80') {
          len = 0;
          return !nullable;
        }
        c = *gptr_++;
        len = 1;
        if (c == '\x80') {
          len -=  nullable;
          return true;
        }
        else if (nullable && c == '\x00' && *gptr_++ == '\x80')
          return true;
        BOOST_THROW_EXCEPTION(fast_dynamic_error("D9"));
      }

      len = get_entity_length();
      gptr_ += len;
      return true;
    }

    /**
     * Decode an unicode string.
     *
     * @param [out] bv The decoded unicode string
     * @param [out] len The decoded unicode string length
     * @param [in] instruction
     * @return false if the decoded value is null
     **/
    bool decode(const char*&                   bv,
                uint32_t&                      len,
                bool                           nullable,
                const unicode_field_instruction* /* instruction */)
    {
      if (this->decode(len, nullable))
      {
        bv = gptr_;
        gptr_ += len;
        return true;
      }
      return false;
    }

    /**
     * Decode a byte vector.
     *
     * @param [out] bv The decoded byte vector
     * @param [out] len The decoded byte vector length
     * @param [in] instruction
     * @return false if the decoded value is null
     **/
    bool decode(const unsigned char*&              bv,
                uint32_t&                          len,
                bool                               nullable,
                const byte_vector_field_instruction* /* instruction */)
    {
      if (this->decode(len, nullable))
      {
        bv = reinterpret_cast<const unsigned char*>(gptr_);
        gptr_ += len;
        return true;
      }
      return false;
    }

    void gbump (int n)
    {
      gptr_ += n;
    }

#ifdef REPORT_OVERFLOW
    std::ostream& overflow_log ()
    {
      return *overflow_log_;
    }
    std::ostream* overflow_log_;
#endif
  
  private:

    unsigned char sbumpc()
    {
      if (in_avail() < 1)
        BOOST_THROW_EXCEPTION(fast_dynamic_error("Buffer underflow"));
      return *(gptr_++);
    }

    const char*gptr_, *egptr_;
    friend std::ostream& operator << (std::ostream& os, const fast_istream& istream);
};


namespace detail {

template <typename T>
struct int_trait;


template <>
struct int_trait<int8_t>   // only used for decoding decimal exponent
{
  typedef int8_t temp_type;
};


template <>
struct int_trait<int32_t>
{
  typedef int32_t temp_type;
};

template <>
struct int_trait<uint32_t>
{
  typedef uint64_t temp_type;
};

template <>
struct int_trait<int64_t>
{
  typedef int64_t temp_type;
};

struct temp_uint64_t
{
  uint64_t value;
  bool carry;

  temp_uint64_t(int)
    : value(0), carry(false)
  {
  }

  temp_uint64_t& operator = (uint64_t other)
  {
    value = other;
    return *this;
  }

  temp_uint64_t& operator |= (uint64_t other)
  {
    value |= other;
    return *this;
  }

  temp_uint64_t& operator <<= (int /* shiftbits */)     // shiftbits should always be 7
  {
    carry =  (value == 0x0200000000000000ULL);
    value <<= 7;
    return *this;
  }

  operator uint64_t () const {
    return value;
  }
};

template <typename T>
inline bool to_nullable_value(T& result, typename int_trait<T>::temp_type tmp)
{
  if (tmp == 0) {
    return false;
  }
  else if (tmp > 0)
    result = static_cast<T>(tmp -1);
  else
    result = static_cast<T>(tmp);
  return true;
}

template <>
struct int_trait<uint64_t>
{
  typedef temp_uint64_t temp_type;
};

inline bool to_nullable_value(uint64_t& result, temp_uint64_t tmp)
{
  if (tmp.value == 0) {
    if (!tmp.carry)
      return false;
    else {
      result = std::numeric_limits<uint64_t>::max();
    }
  }
  else if (tmp.value > 0)
    result = tmp.value -1;
  else
    result = tmp.value;
  return true;

}

}

template <typename T>
typename boost::enable_if< boost::is_integral<T> ,bool>::type
fast_istream::decode(T& result, bool nullable)
{
  typename detail::int_trait<T>::temp_type tmp = 0;

  char c = sbumpc();

  if (boost::is_unsigned<T>::value) {
    tmp = c & 0x7F;
  }
  else {
    if (c & 0x40) {
      // this is a signed integer
      tmp = (static_cast<T>(-1) << 7) | (c & 0x7F);
    }
    else {
      // unsigned integer, mask the most significat two bits
      tmp = c & 0x3F;
    }
  }

  int consumed_bytes = 0;

  const int max_bytes = 8*sizeof(T)/7+1;

  while ( (c & 0x80) == 0 && consumed_bytes < max_bytes) {
    tmp <<= 7;
    c = sbumpc();
    tmp |= ( c & 0x7F );
    ++consumed_bytes;
  };

  if ((c & 0x80) ==0)
    BOOST_THROW_EXCEPTION(fast_dynamic_error("D2"));

  if (nullable) {
    return detail::to_nullable_value(result, tmp);
  }
  else {
    result = static_cast<T>(tmp);
  }
  return true;
}

}

#endif /* end of include guard: FAST_ISTREAM_H_7X1JL4X6 */
