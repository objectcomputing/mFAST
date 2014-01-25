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
#ifndef ENCODER_PRESENCE_MAP_H_MQSBLA37
#define ENCODER_PRESENCE_MAP_H_MQSBLA37

#include <boost/detail/endian.hpp>
#include "fast_ostream.h"


namespace mfast
{

  class encoder_presence_map
  {
  public:
    encoder_presence_map();
    void init(fast_ostream* stream, std::size_t maxbits);
    // returns true if the presence_map is full and needed to be serialized.
    void set_next_bit(bool v);
    void commit();

  private:

    void reset();

    std::size_t nbytes_;
    uint64_t value_, mask_;
    fast_ostream* stream_;
    std::size_t offset_;
    std::size_t maxbytes_;
  };

  inline
  encoder_presence_map::encoder_presence_map()
    : stream_(0)
    , offset_(0)
    , maxbytes_(0)
  {
    reset();
  }

  inline void
  encoder_presence_map::init(fast_ostream* stream, std::size_t maxbits)
  {
    stream_ = stream;
    offset_ = stream->offset();
    maxbytes_ =  (maxbits +6)/7; // i.e. ceiling(maxbits/7)
    stream_->buf_->skip(maxbytes_);
  }

  inline void
  encoder_presence_map::reset()
  {
    nbytes_ = 0;
    value_ = 0;

#ifdef BOOST_BIG_ENDIAN
    mask_ =  0x80ULL << 56;
#else
    mask_ =  0x80ULL;
#endif
  }

  inline void
  encoder_presence_map::commit()
  {
#ifdef BOOST_BIG_ENDIAN
    const uint64_t stop_bit_mask = (0x8000000000000000ULL >> (nbytes_ * 8));
#else
    const uint64_t stop_bit_mask = (0x80ULL << (nbytes_ * 8));
#endif

    value_ |= stop_bit_mask;
    stream_->write_bytes_at(&value_, ++nbytes_, offset_, true);
  }

  inline void
  encoder_presence_map::set_next_bit(bool v)
  {
    if ( (mask_ & 0x0101010101010101ULL) != 0) {
#ifdef BOOST_BIG_ENDIAN
      mask_ >>= 2;
#else
      mask_ <<= 14;
#endif
      ++nbytes_;
    }
    else {
      mask_ >>= 1;
    }

    if (mask_ == 0) {
      // we need to commit the current pmap before preceed
      stream_->write_bytes_at(&value_, 8, offset_, false);
      offset_ += 8;
      maxbytes_ -= 8;
      reset();
      mask_ >>= 1;
    }

    if (v) {
      value_ |= mask_;
    }
  }

}

#endif /* end of include guard: ENCODER_PRESENCE_MAP_H_MQSBLA37 */
