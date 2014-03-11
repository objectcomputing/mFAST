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
#ifndef FAST_ISTREAMBUF_H_7X1JL4X6
#define FAST_ISTREAMBUF_H_7X1JL4X6
#include <stdexcept>

#include "mfast/exceptions.h"
#include <iostream>

namespace mfast
{

  class fast_istream;
  class decoder_presence_map;

  class fast_istreambuf
  {
  public:
    fast_istreambuf(const char* buf, std::size_t sz)
      : gptr_(buf)
      , egptr_(buf+sz)
    {
    }

    size_t in_avail() const
    {
      return egptr_-gptr_;
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

  protected:
    friend class fast_istream;
    friend class decoder_presence_map;
    friend class fast_decoder;

    void gbump (std::ptrdiff_t n)
    {
      gptr_ += n;
    }

    unsigned char sbumpc()
    {
      if (in_avail() < 1)
        BOOST_THROW_EXCEPTION(fast_dynamic_error("Buffer underflow"));
      return *(gptr_++);
    }

    char sgetc() const
    {
      return *gptr_;
    }

    const char* gptr()
    {
      return gptr_;
    }

    const char* egptr()
    {
      return egptr_;
    }

    const char*gptr_, *egptr_;
  };



}

#endif /* end of include guard: FAST_ISTREAMBUF_H_7X1JL4X6 */
