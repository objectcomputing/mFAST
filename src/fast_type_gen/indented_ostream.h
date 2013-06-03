// Copyright (c) 2013, Huang-Ming Huang,  Object Computing, Inc.
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
#ifndef INDENTED_OSTREAM_H_TJD6GPXA
#define INDENTED_OSTREAM_H_TJD6GPXA

#include <ostream>
#include <iomanip>
#include <sstream>
struct indent_t {};

template <typename OSTREAM>
class indented_ostream : public OSTREAM
{
  public:
    indented_ostream()
      : indent_level_(0)
    {
    }

    void inc_indent(std::size_t level=1)
    {
      indent_level_ += level;
    }

    void dec_indent(std::size_t level=1)
    {
      indent_level_ -= level;
    }

    void reset_indent(std::size_t level=0)
    {
      indent_level_ = 0;
    }

    std::size_t indent_level() const
    {
      return indent_level_;
    }
  private:
    std::size_t indent_level_;
};


inline indented_ostream<std::stringstream>&
operator << (indented_ostream<std::stringstream>& strm, indent_t)
{
  if (strm.indent_level() > 0)
    strm << std::setw(strm.indent_level()*2) << ' ';
  return strm;
}

template <typename T>
inline indented_ostream<std::stringstream>&
operator << (indented_ostream<std::stringstream>& strm, const T& t)
{
  static_cast<std::ostream&>(strm) << t;
  return strm;
}



#endif /* end of include guard: INDENTED_OSTREAM_H_TJD6GPXA */
