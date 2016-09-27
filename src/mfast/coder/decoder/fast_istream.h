// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.
#pragma once

#include <limits>

#include "../../instructions/string_instructions.h"
#include "../../instructions/byte_vector_instruction.h"
#include "fast_istreambuf.h"
#include "decoder_presence_map.h"

namespace mfast {
class fast_istream;
std::ostream &operator<<(std::ostream &os, const fast_istream &istream);
class fast_istream {
public:
  fast_istream(fast_istreambuf *sb);

  void reset(fast_istreambuf *sb);

  bool eof() const { return buf_->in_avail() == 0; }
  bool operator!() const { return buf_->in_avail() == 0; }
  /**
   * Decode an integer and store the result in @a result.
   *
   * @return false if the decoded value is null
   **/
  template <typename T, typename Nullable>
  enable_if_t<std::is_integral<T>::value, bool> decode(T &result,
                                                       Nullable nullable);

  void decode(decoder_presence_map &pmap) {
    if (!pmap.load(*buf_)) {
      while (0 == (buf_->sbumpc() & 0x80))
        ;
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
  template <typename Nullable>
  bool decode(const char *&ascii, uint32_t &len,
              const ascii_field_instruction * /* instruction */,
              Nullable nullable) {
    char c = buf_->sgetc();
    ascii = buf_->gptr();
    if ((c & '\x7F') == 0) {
      buf_->gbump(1);

      if (c == '\x80') {
        len = 0;
        return !nullable;
      }
      c = buf_->sgetc();
      buf_->gbump(1);

      len = 1;
      if (c == '\x80') {
        len -= nullable;
        return true;
      } else if (nullable && c == '\x00') {
        c = buf_->sgetc();
        buf_->gbump(1);
        if (c == '\x80')
          return true;
      }
      BOOST_THROW_EXCEPTION(fast_dynamic_error("D9"));
    }

    len = static_cast<uint32_t>(buf_->get_entity_length());
    buf_->gbump(len);
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
  template <typename Nullable>
  bool decode(const char *&bv, uint32_t &len,
              const unicode_field_instruction * /* instruction */,
              Nullable nullable) {
    if (this->decode(len, nullable)) {
      bv = buf_->gptr();
      buf_->gbump(len);
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
  template <typename Nullable>
  bool decode(const unsigned char *&bv, uint32_t &len,
              const byte_vector_field_instruction * /* instruction */,
              Nullable nullable) {
    if (this->decode(len, nullable)) {
      bv = reinterpret_cast<const unsigned char *>(buf_->gptr());
      buf_->gbump(len);
      return true;
    }
    return false;
  }

  std::ostream &warning_log() { return *warning_log_; }
  void warning_log(std::ostream *log) { warning_log_ = log; }

private:
  friend std::ostream &operator<<(std::ostream &os,
                                  const fast_istream &istream);

  const char *gptr() const { return buf_->gptr_; }
  const char *egptr() const { return buf_->egptr_; }
  fast_istreambuf *buf_;
  std::ostream *warning_log_;
};

namespace detail {
template <typename T> struct int_trait;

template <>
struct int_trait<int16_t> // only used for decoding decimal exponent
{
  typedef int16_t temp_type;
};

template <> struct int_trait<int32_t> { typedef int32_t temp_type; };

template <> struct int_trait<uint32_t> { typedef uint64_t temp_type; };

template <> struct int_trait<int64_t> { typedef int64_t temp_type; };

struct temp_uint64_t {
  uint64_t value;
  bool carry;

  temp_uint64_t(int) : value(0), carry(false) {}
  temp_uint64_t &operator=(uint64_t other) {
    value = other;
    return *this;
  }

  temp_uint64_t &operator|=(uint64_t other) {
    value |= other;
    return *this;
  }

  temp_uint64_t &
  operator<<=(int /* shiftbits */) // shiftbits should always be 7
  {
    carry = (value == 0x0200000000000000ULL);
    value <<= 7;
    return *this;
  }

  operator uint64_t() const { return value; }
  uint64_t operator-(int x) const { return value - x; }
};

template <typename T>
inline bool to_nullable_value(T &result, typename int_trait<T>::temp_type tmp,
                              int decrement_value) {
  if (tmp == 0) {
    return false;
  }
  result = static_cast<T>(tmp - decrement_value);
  return true;
}

template <> struct int_trait<uint64_t> { typedef temp_uint64_t temp_type; };

inline bool to_nullable_value(uint64_t &result, temp_uint64_t tmp,
                              int decrement_value) {
  if (tmp.value == 0) {
    if (!tmp.carry)
      return false;
    else {
      result = std::numeric_limits<uint64_t>::max();
    }
  } else
    result = tmp - decrement_value;
  return true;
}
}

inline fast_istream::fast_istream(fast_istreambuf *sb)
    : buf_(sb), warning_log_(nullptr) {}
inline void fast_istream::reset(fast_istreambuf *sb) { buf_ = sb; }
template <typename T, typename Nullable>
enable_if_t<std::is_integral<T>::value, bool>
fast_istream::decode(T &result, Nullable nullable) {
  typename detail::int_trait<T>::temp_type tmp = 0;

  char c = buf_->sbumpc();
  // bool decrement_value = true;
  int decrement_value = nullable;
  if (std::is_unsigned<T>::value) {
    tmp = c & 0x7F;
  } else {
    if (c & 0x40) {
      // this is a negative integer
      decrement_value = 0;
      tmp = (static_cast<T>(-1) ^ 0x7F ) | (c & 0x7F);
    } else {
      // positive integer, mask the most significant two bits
      tmp = c & 0x3F;
    }
  }

  while ((c & 0x80) == 0) {
    tmp <<= 7;
    c = buf_->sbumpc();
    tmp |= (c & 0x7F);
  };

  if (nullable) {
    return detail::to_nullable_value(result, tmp, decrement_value);
  } else {
    result = static_cast<T>(tmp);
  }
  return true;
}
}
