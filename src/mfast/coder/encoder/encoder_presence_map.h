// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.
#pragma once

#include <boost/detail/endian.hpp>
#include "fast_ostream.h"

namespace mfast {
#ifdef BOOST_BIG_ENDIAN
const int SMALLEST_ADDRESS_BYTE = sizeof(std::size_t) - 1;
#else
const int SMALLEST_ADDRESS_BYTE = 0;
#endif

const std::size_t init_mask = static_cast<std::size_t>(1)
                              << ((SMALLEST_ADDRESS_BYTE * 8) + 7);

class encoder_presence_map {
public:
  encoder_presence_map();
  void init(fast_ostream *stream, std::size_t maxbits);
  // returns true if the presence_map is full and needed to be serialized.
  void set_next_bit(bool v);
  void commit();

private:
  void reset();

  std::size_t nbytes_;
  std::size_t value_, mask_;
  fast_ostream *stream_;
  std::size_t offset_;
  std::size_t maxbytes_;
};

inline encoder_presence_map::encoder_presence_map()
    : stream_(nullptr), offset_(0), maxbytes_(0) {
  reset();
}

inline void encoder_presence_map::init(fast_ostream *stream,
                                       std::size_t maxbits) {
  stream_ = stream;
  offset_ = stream->offset();
  maxbytes_ = (maxbits + 6) / 7; // i.e. ceiling(maxbits/7)
  stream_->buf_->skip(maxbytes_);
}

inline void encoder_presence_map::reset() {
  nbytes_ = 0;
  value_ = 0;
  mask_ = init_mask;
}

inline void encoder_presence_map::commit() {
#ifdef BOOST_BIG_ENDIAN
  const std::size_t stop_bit_mask = (init_mask >> (nbytes_ * 8));
#else
  const std::size_t stop_bit_mask = (init_mask << (nbytes_ * 8));
#endif

  value_ |= stop_bit_mask;
  stream_->write_bytes_at(&value_, ++nbytes_, offset_, true);
}

inline BOOST_CONSTEXPR std::size_t get_next_bit_mask(std::size_t i) {
  return i == 0 ? 0 : (get_next_bit_mask(i - 1) << 8) | 0x01;
}

inline void encoder_presence_map::set_next_bit(bool v) {
  const std::size_t next_bit_mask = get_next_bit_mask(sizeof(std::size_t));

  if ((mask_ & next_bit_mask) != 0) {
#ifdef BOOST_BIG_ENDIAN
    mask_ >>= 2;
#else
    mask_ <<= 14;
#endif
    ++nbytes_;
  } else {
    mask_ >>= 1;
  }

  if (mask_ == 0) {
    // we need to commit the current pmap before preceed
    stream_->write_bytes_at(&value_, sizeof(std::size_t), offset_, false);
    offset_ += sizeof(std::size_t);
    maxbytes_ -= sizeof(std::size_t);
    reset();
    mask_ >>= 1;
  }

  if (v) {
    value_ |= mask_;
  }
}
}
