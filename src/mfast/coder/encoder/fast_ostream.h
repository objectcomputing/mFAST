// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.
#pragma once

#include "../../field_instructions.h"
#include "../common/codec_helper.h"
#include "fast_ostreambuf.h"

namespace mfast {
class encoder_presence_map;
class fast_ostream : private detail::codec_helper {
public:
  fast_ostream(allocator *alloc);

  fast_ostreambuf *rdbuf() const;
  fast_ostreambuf *rdbuf(fast_ostreambuf *sb);

  template <typename IntType, typename Nullable>
  void encode(IntType t, bool is_null, Nullable nullable);

  template <typename Nullable>
  void encode(const char *ascii, uint32_t len,
              const ascii_field_instruction * /* instruction */,
              Nullable nullable);

  template <typename Nullable>
  void encode(const char *unicode, uint32_t len,
              const unicode_field_instruction * /* instruction */,
              Nullable nullable);

  template <typename Nullable>
  void encode(const unsigned char *bv, uint32_t len,
              const byte_vector_field_instruction * /* instruction */,
              Nullable nullable);

  void encode_null();

  template <typename T> void save_previous_value(const T &cref) const;

  void allow_overlong_pmap(bool v);

private:
  friend class encoder_presence_map;

  void write_bytes_at(std::size_t *bytes, std::size_t nbytes,
                      std::size_t offset, bool pmap_end);

  std::size_t offset() const { return rdbuf()->length(); }
  fast_ostreambuf *buf_;
  allocator *alloc_;
  bool allow_overlong_pmap_;
};

inline fast_ostream::fast_ostream(allocator *alloc)
    : alloc_(alloc), allow_overlong_pmap_(true) {}
inline fast_ostreambuf *fast_ostream::rdbuf() const { return buf_; }
inline fast_ostreambuf *fast_ostream::rdbuf(fast_ostreambuf *sb) {
  buf_ = sb;
  return buf_;
}

namespace detail {
template <typename T>
inline enable_if_t<std::is_signed<T>::value, bool> is_positive(T v) {
  return v >= 0;
}

template <typename T>
inline enable_if_t<std::is_unsigned<T>::value, bool> is_positive(T) {
  return true;
}

// use non-const reference to avoid ambiguious overloading problem
inline bool encode_max_value(uint64_t &value, fast_ostreambuf *buf) {
  if (value == (std::numeric_limits<uint64_t>::max)()) {
    buf->sputn("\x02\x00\x00\x00\x00\x00\x00\x00\x00\x80", 10);
    return true;
  }
  return false;
}

inline bool encode_max_value(int64_t &value, fast_ostreambuf *buf) {
  if (value == (std::numeric_limits<int64_t>::max)()) {
    buf->sputn("\x01\x00\x00\x00\x00\x00\x00\x00\x00\x80", 10);
    return true;
  }
  return false;
}

inline bool encode_max_value(uint32_t &value, fast_ostreambuf *buf) {
  if (value == (std::numeric_limits<uint32_t>::max)()) {
    buf->sputn("\x10\x00\x00\x00\x80", 5);
    return true;
  }
  return false;
}

inline bool encode_max_value(int32_t &value, fast_ostreambuf *buf) {
  if (value == (std::numeric_limits<int32_t>::max)()) {
    buf->sputn("\x08\x00\x00\x00\x80", 5);
    return true;
  }
  return false;
}

inline bool encode_max_value(int16_t &, fast_ostreambuf *) { return false; }
}

template <typename IntType, typename Nullable>
void fast_ostream::encode(IntType value, bool is_null, Nullable nullable) {
  if (nullable) {
    if (is_null) {
      rdbuf()->sputc('\x80');
      return;
    } else if (detail::encode_max_value(value, rdbuf())) {
      return;
    } else if (detail::is_positive(value)) {
      ++value;
    }
  } else if (value == 0) {
    rdbuf()->sputc('\x80');
    return;
  }

  bool positive = detail::is_positive(value);
  // padding_mask is used to pad highest 7 bits to 1 when t is negative after t
  // is shift by 7
  // this ensure that t has no significat bits at the highest 7 bits after
  // shifting
  const IntType padding_mask =
      (static_cast<IntType>(value > 0 ? 0 : 0xFE) << (sizeof(IntType) - 1) * 8);
  const IntType no_significant_bits = (value > 0 ? 0 : -1);

  const unsigned max_encoded_length = sizeof(IntType) * 8 / 7 + 1;
  char buffer[max_encoded_length + 1] = {'\0'};
  int i = max_encoded_length - 1;

  for (; i >= 0 && value != no_significant_bits; --i) {
    buffer[i] = value & static_cast<IntType>(0x7F);
    value = (value >> 7) | padding_mask;
  }

  ++i;

  if (std::is_signed<IntType>::value) {
    if (positive) {
      // check if the sign bit is on
      if (buffer[i] & 0x40) {
        // signed bit is on and this is positive integer, we need to pad an
        // extra byte \x00
        // in the front
        buffer[--i] = '\x00';
      }
    } else if ((buffer[i] & 0x40) == 0) {
      // this is negative integer and yet the sign bit is off, we need to pad an
      // extra byte
      // \x7F in the front
      buffer[--i] = '\x7F';
    }
  }

  buffer[max_encoded_length - 1] |= 0x80; // stop bit
  rdbuf()->sputn(buffer + i, max_encoded_length - i);
}

template <typename Nullable>
inline void
fast_ostream::encode(const char *ascii, uint32_t len,
                     const ascii_field_instruction * /* instruction */,
                     Nullable nullable) {
  assert(ascii || nullable);

  if (nullable && ascii == nullptr) {
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
    } else {
      rdbuf()->sputn("\x00\x80", 2);
    }
    return;
  }

  rdbuf()->sputn(ascii, len - 1);
#ifdef _MSC_VER
#pragma warning(suppress : 6011)
#endif
  rdbuf()->sputc(ascii[len - 1] | 0x80);
}

template <typename Nullable>
inline void
fast_ostream::encode(const char *unicode, uint32_t len,
                     const unicode_field_instruction * /* instruction */,
                     Nullable nullable) {
  // non-nullable string cannot have a null value
  assert(unicode || nullable);

  encode(len, unicode == nullptr, nullable);
  if (unicode && len > 0) {
    rdbuf()->sputn(unicode, len);
  }
}

template <typename Nullable>
inline void
fast_ostream::encode(const unsigned char *bv, uint32_t len,
                     const byte_vector_field_instruction * /* instruction */,
                     Nullable nullable) {
  // non-nullable byteVector cannot have a null value
  assert(bv || nullable);

  encode(len, bv == nullptr, nullable);
  if (bv && len > 0) {
    rdbuf()->sputn(reinterpret_cast<const char *>(bv), len);
  }
}

template <typename T>
inline void fast_ostream::save_previous_value(const T &cref) const {
  value_storage &s = previous_value_of(cref);
  typename mref_of<T>::type prev_mref(alloc_, &s, cref.instruction());
  prev_mref.as(cref);
  s.defined(true);
}

inline void fast_ostream::write_bytes_at(std::size_t *bytes, std::size_t nbytes,
                                         std::size_t offset, bool pmap_end) {
  rdbuf()->write_bytes_at(reinterpret_cast<const char *>(bytes), nbytes, offset,
                          pmap_end && !allow_overlong_pmap_);
}

inline void fast_ostream::encode_null() { rdbuf()->sputc('\x80'); }
inline void fast_ostream::allow_overlong_pmap(bool v) {
  allow_overlong_pmap_ = v;
}
}
