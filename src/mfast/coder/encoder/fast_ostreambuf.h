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

#ifndef FAST_OSTREAMBUF_H_TWEMCH8
#define FAST_OSTREAMBUF_H_TWEMCH8

#include <stdexcept>
#include "mfast/exceptions.h"

namespace mfast
{

  class buffer_overflow_error
    : public virtual boost::exception, public virtual std::runtime_error
  {
  public:
    buffer_overflow_error()
      : std::runtime_error("buffer overflow")
    {
    }

  };

  class fast_ostreambuf
  {
  public:
    fast_ostreambuf(char* buf, std::size_t size);

    template <std::size_t SIZE>
    fast_ostreambuf(char (&array)[SIZE]);

    void sputc(char c);
    void sputn(const char* data, std::size_t n);
    void skip(std::size_t n);

    virtual std::size_t length() const;
    virtual void write_bytes_at(const char* data, std::size_t n, std::size_t offset, bool shrink);

    const char* pbase() const
    {
      return pbase_;
    }

  protected:
    virtual void overflow(std::size_t n);
    void setp(char* pbase, char* pptr, char* epptr);
    char* pbase_, *pptr_, *epptr_;
  };

  inline
  fast_ostreambuf::fast_ostreambuf(char* buf, std::size_t size)
    : pbase_(buf)
    , pptr_(buf)
    , epptr_(buf+size)
  {
  }

  template <std::size_t SIZE>
  fast_ostreambuf::fast_ostreambuf(char (&array)[SIZE])
    : pbase_(array)
    , pptr_(array)
    , epptr_(array+SIZE)
  {
  }

  inline void
  fast_ostreambuf::sputc(char c)
  {
    while (pptr_ >= epptr_)
      overflow(1);

    *pptr_ = c;
    ++pptr_;
  }

  inline void
  fast_ostreambuf::sputn(const char* data, std::size_t n)
  {
    while (pptr_+n >= epptr_)
      overflow(n);

    std::copy(data, data+n, pptr_);
    pptr_ += n;
  }

  inline void
  fast_ostreambuf::skip(std::size_t n)
  {
    while (pptr_+n >= epptr_)
      overflow(n);
    pptr_ += n;
  }

  inline void
  fast_ostreambuf::setp(char* pbase, char* pptr, char* epptr)
  {
    pbase_ = pbase;
    pptr_ = pptr;
    epptr_ = epptr;
  }

}

#endif /* end of include guard: FAST_OSTREAMBUF_H_TWEMCH8 */
