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

#ifndef RESIZABLE_FAST_OSTREAMBUF_H_ZY8FH19T
#define RESIZABLE_FAST_OSTREAMBUF_H_ZY8FH19T

#include "fast_ostreambuf.h"
#include <vector>

namespace mfast {

  class resizable_fast_ostreambuf
    : public fast_ostreambuf
  {
  public:
    resizable_fast_ostreambuf(std::vector<char>& buf)
      : fast_ostreambuf(0, 0)
      , buf_(buf)
    {
      std::size_t old_size = buf.size();
      std::size_t new_size = old_size + 1024;
      buf.resize(new_size);
      char* addr = &buf_[0];
      setp(addr,addr+old_size, addr+new_size);

    }

    virtual void overflow(std::size_t n)
    {
      std::size_t len = length();
      buf_.resize(2*(len + n));
      char* addr = &buf_[0];
      setp(addr,addr+len, addr+buf_.size());
    }

  private:
    std::vector<char>& buf_;
  };

}

#endif /* end of include guard: RESIZABLE_FAST_OSTREAMBUF_H_ZY8FH19T */
