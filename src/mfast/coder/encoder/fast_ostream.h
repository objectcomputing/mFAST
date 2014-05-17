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

#ifndef FAST_OSTREAM_H_DUY5XTNJ
#define FAST_OSTREAM_H_DUY5XTNJ

#include "mfast/field_instructions.h"
#include "../common/codec_helper.h"
#include "fast_ostreambuf.h"

namespace mfast {

  class encoder_presence_map;
  class fast_ostream
    : private detail::codec_helper
  {
  public:
    fast_ostream(allocator* alloc);

    fast_ostreambuf* rdbuf () const;
    fast_ostreambuf* rdbuf (fast_ostreambuf* sb);

    template <typename IntType>
    void encode(IntType t, bool nullable, bool is_null = false);

    void encode(const char* ascii,
                uint32_t    len,
                bool        nullable,
                const ascii_field_instruction* /* instruction */);

    void encode(const char* unicode,
                uint32_t    len,
                bool        nullable,
                const unicode_field_instruction* /* instruction */);

    void encode(const unsigned char* bv,
                uint32_t             len,
                bool                 nullable,
                const byte_vector_field_instruction* /* instruction */);


    void encode_null();

    template <typename T>
    void save_previous_value(const T& cref) const;

    void allow_overlong_pmap(bool v);

  private:
    friend class encoder_presence_map;

    void write_bytes_at(uint64_t* bytes, std::size_t nbytes, std::size_t offset, bool pmap_end);

    std::size_t offset() const
    {
      return rdbuf()->length();
    }

    fast_ostreambuf* buf_;
    allocator* alloc_;
    bool allow_overlong_pmap_;
  };

  inline
  fast_ostream::fast_ostream(allocator* alloc)
    : alloc_(alloc)
    , allow_overlong_pmap_(true)
  {
  }

  inline fast_ostreambuf*
  fast_ostream::rdbuf () const
  {
    return buf_;
  }

  inline fast_ostreambuf*
  fast_ostream::rdbuf (fast_ostreambuf* sb)
  {
    buf_ = sb;
    return buf_;
  }

  namespace detail {

    template <typename T>
    inline typename boost::enable_if<boost::is_signed<T>, bool>::type
    is_positive(T v)
    {
      return v >= 0;
    }

    template <typename T>
    inline typename boost::enable_if<boost::is_unsigned<T>, bool>::type
    is_positive(T)
    {
      return true;
    }

// use non-const reference to avoid ambiguious overloading problem
    inline bool encode_max_value(uint64_t& value, fast_ostreambuf* buf)
    {
      if (value == (std::numeric_limits<uint64_t>::max)()) {
        buf->sputn("\x02\x00\x00\x00\x00\x00\x00\x00\x00\x80", 10);
        return true;
      }
      return false;
    }

    inline bool encode_max_value(int64_t& value, fast_ostreambuf* buf)
    {
      if (value == (std::numeric_limits<int64_t>::max)()) {
        buf->sputn("\x01\x00\x00\x00\x00\x00\x00\x00\x00\x80", 10);
        return true;
      }
      return false;
    }

    inline bool encode_max_value(uint32_t& value, fast_ostreambuf* buf)
    {
      if (value == (std::numeric_limits<uint32_t>::max)()) {
        buf->sputn("\x10\x00\x00\x00\x80", 5);
        return true;
      }
      return false;
    }

    inline bool encode_max_value(int32_t& value, fast_ostreambuf* buf)
    {
      if (value == (std::numeric_limits<int32_t>::max)()) {
        buf->sputn("\x08\x00\x00\x00\x80", 5);
        return true;
      }
      return false;
    }

    inline bool encode_max_value(int16_t&, fast_ostreambuf*)
    {
      return false;
    }

  }

  template <typename IntType>
  void fast_ostream::encode(IntType value, bool nullable, bool is_null)
  {
    if (nullable) {
      if (is_null) {
        rdbuf()->sputc('\x80');
        return;
      }
      else if (detail::encode_max_value(value, rdbuf())) {
        return;
      }
      else if ( detail::is_positive(value) ) {
        ++value;
      }
    }
    else if (value == 0) {
      rdbuf()->sputc('\x80');
      return;
    }

    bool positive = detail::is_positive(value);
    // padding_mask is used to pad highest 7 bits to 1 when t is negative after t is shift by 7
    // this ensure that t has no significat bits at the highest 7 bits after shifting
    const IntType padding_mask = (static_cast<IntType>( value > 0 ? 0 : 0xFE ) << (sizeof(IntType) -1)*8);
    const IntType no_significant_bits = (value > 0 ? 0 : -1);

    const unsigned max_encoded_length= sizeof(IntType)*8/7+1;
    char buffer[max_encoded_length]= {'\0'};
    int i = max_encoded_length-1;

    for (; i >= 0 && value != no_significant_bits; --i) {
      buffer[i] = value & static_cast<IntType>(0x7F);
      value = (value >> 7) | padding_mask;
    }

    ++i;

    if (boost::is_signed<IntType>::value) {
      if (positive) {
        // check if the sign bit is on
        if (buffer[i] & 0x40 ) {
          // signed bit is on and this is positive integer, we need to pad an extra byte \x00 in the front
          buffer[--i] = '\x00';
        }
      }
      else if ((buffer[i] & 0x40) == 0) {
        // this is negative integer and yet the sign bit is off, we need to pad an extra byte \x7F in the front
        buffer[--i] = '\x7F';
      }
    }

    buffer[max_encoded_length-1] |= 0x80; // stop bit
    rdbuf()->sputn(buffer+i, max_encoded_length-i);
  }

  inline void
  fast_ostream::encode(const char* ascii,
                       uint32_t    len,
                       bool        nullable,
                       const ascii_field_instruction* /* instruction */)
  {
    assert( ascii || nullable );

    if (nullable && ascii==0) {
      rdbuf()->sputc('\x80');
      return;
    }

    if (len == 0) {
      if (nullable) {
        rdbuf()->sputc('\x00');
      }
      rdbuf()->sputc('\x80');
      return;
    }

    if (len == 1 && ascii[0] == '\x00') {
      if (nullable) {
        rdbuf()->sputn("\x00\x00\x80", 3);
      }
      else {
        rdbuf()->sputn("\x00\x80", 2);
      }
      return;
    }


    rdbuf()->sputn(ascii, len-1);
    rdbuf()->sputc(ascii[len-1] | 0x80);
  }

  inline void
  fast_ostream::encode(const char* unicode,
                       uint32_t    len,
                       bool        nullable,
                       const unicode_field_instruction* /* instruction */)
  {
    // non-nullable string cannot have a null value
    assert( unicode || nullable );

    encode(len, nullable, unicode == 0);
    if (unicode && len > 0) {
      rdbuf()->sputn(unicode, len);
    }
  }

  inline void
  fast_ostream::encode(const unsigned char* bv,
                       uint32_t             len,
                       bool                 nullable,
                       const byte_vector_field_instruction* /* instruction */)
  {
    // non-nullable byteVector cannot have a null value
    assert( bv || nullable );

    encode(len, nullable, bv == 0);
    if (bv && len > 0) {
      rdbuf()->sputn(reinterpret_cast<const char*>(bv), len);
    }
  }

  template <typename T>
  inline void
  fast_ostream::save_previous_value(const T& cref) const
  {
    value_storage& s = previous_value_of(cref);
    typename mref_of<T>::type prev_mref(alloc_, &s, cref.instruction());
    prev_mref.as(cref);
    s.defined(true);
  }

  inline void
  fast_ostream::write_bytes_at(uint64_t* bytes, std::size_t nbytes, std::size_t offset, bool pmap_end)
  {
    rdbuf()->write_bytes_at(reinterpret_cast<const char*>(bytes), nbytes, offset, pmap_end && !allow_overlong_pmap_);
  }

  inline void
  fast_ostream::encode_null()
  {
    rdbuf()->sputc('\x80');
  }

  inline void
  fast_ostream::allow_overlong_pmap(bool v)
  {
    allow_overlong_pmap_ = v;
  }

}

#endif /* end of include guard: FAST_OSTREAM_H_DUY5XTNJ */
