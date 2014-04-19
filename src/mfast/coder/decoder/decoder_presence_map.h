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
#ifndef DECODER_PRESENCE_MAP_H_MWSEB461
#define DECODER_PRESENCE_MAP_H_MWSEB461
#include <stdint.h>
#include "fast_istreambuf.h"

namespace mfast {

  class decoder_presence_map
  {
  public:
    decoder_presence_map()
      : cur_bitmap_ (0)
      , mask_(0)
      , continue_(0)
    {
    }

    bool is_next_bit_set()
    {
      mask_ >>=1;
      if (mask_ == 0 && continue_) {
        load(continue_);
      }
      bool result = (cur_bitmap_ & mask_) != 0;
      return result;
    }

    bool load(fast_istreambuf& buf)
    {
      const char* addr = buf.gptr();
      bool result = load(addr);
      buf.gbump(addr-buf.gptr());
      return result;
    }

    // only used for test case verification
    uint64_t mask() const
    {
      return mask_;
    }

  private:

    bool load(const char*& addr)
    {
      bool load_complete = false;
      const int max_load_byes = sizeof(uint64_t)*8/7;
      mask_ = 1;
      int i;
      for (i = 0; i < max_load_byes && !load_complete; ++i, ++addr)
      {
        char c = *addr;
        cur_bitmap_ <<= 7;
        cur_bitmap_ |= c & '\x7F';
        mask_ <<= 7;
        if ('\x80' == (c & '\x80')) {
          load_complete = true;
        }
      }
      return load_complete;
    }

    uint64_t cur_bitmap_;
    uint64_t mask_;
    const char* continue_;
    friend std::ostream& operator << (std::ostream&, const mfast::decoder_presence_map&);
  };

}

#endif /* end of include guard: DECODER_PRESENCE_MAP_H_MWSEB461 */
