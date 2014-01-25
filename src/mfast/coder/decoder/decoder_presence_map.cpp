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
#include "decoder_presence_map.h"


#ifndef NDEBUG

namespace mfast {
  std::ostream&
  operator << (std::ostream& os, const decoder_presence_map& pmap)
  {
    uint64_t mask = pmap.mask_ >> 1;
    if (mask == 0) {
      os << "0";
      return os;
    }

    os << (mask & pmap.cur_bitmap_ ? "J" : "Q");
    while ((mask >>=1)) {
      os << (mask & pmap.cur_bitmap_ ? "1" : "0");
    }
    return os;
  }

}
#else
// disable MSVC LNK4221
namespace { char dummy; };
#endif
