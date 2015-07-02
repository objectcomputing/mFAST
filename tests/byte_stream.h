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

#pragma once

#include "mfast/coder/encoder/fast_ostreambuf.h"
#include <iostream>
#include <iomanip>
#include <boost/io/ios_state.hpp>


class byte_stream
{
public:
  byte_stream(const char* s, std::size_t len)
    : data_(s)
    , size_(len)
  {
  }

  template <std::size_t SIZE>
  byte_stream(const char (&array) [SIZE])
    : data_(array)
    , size_(SIZE-1)
  {
  }

  byte_stream(const mfast::fast_ostreambuf& sb)
    : data_(sb.pbase())
    , size_(sb.length())
  {
  }

  std::size_t size() const
  {
    return size_;
  }

  const char* data() const
  {
    return data_;
  }

private:
  const char* data_;
  std::size_t size_;
};


inline bool operator == (const byte_stream& lhs, const byte_stream& rhs)
{
  return lhs.size() == rhs.size() && memcmp(lhs.data(), rhs.data(), lhs.size()) == 0;
}

inline bool operator != (const byte_stream& lhs, const byte_stream& rhs)
{
  return !(lhs == rhs);
}

inline std::ostream& operator << (std::ostream& os, const byte_stream& bs )
{
  boost::io::ios_flags_saver ifs( os );

  os << std::hex << std::setfill('0');

  for (std::size_t i = 0; i < bs.size(); ++i)
  {
    os << "\\x" << std::setw(2) <<  (static_cast<uint32_t>(bs.data()[i]) & 0xFF);
  }

  return os;
}


