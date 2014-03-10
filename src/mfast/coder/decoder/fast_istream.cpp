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
// #include "mfast/output.h"
#include "fast_istream.h"

#include <boost/io/ios_state.hpp>

namespace mfast {
  std::ostream&
  operator << (std::ostream& os, const fast_istream& istream)
  {
    const char* ptr = istream.gptr();
    boost::io::ios_flags_saver ifs( os );

    for (int i = 0; i < 2; ++i) {
      // Output at most 2 stop bit encoded entities
      for (; ptr != istream.egptr(); ++ptr) {
        os << std::hex << std::setw(2)<< std::setfill('0') << (static_cast<unsigned>(*ptr) & 0xFF) << " ";
        if (*ptr & '\x80') {
          ++ptr;
          break;
        }
      }
    }

    if (ptr != istream.egptr()) {
      os << " ....";
    }
    return os;
  }

}
