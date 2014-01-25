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
#ifndef INDENTED_OSTREAM_H_TJD6GPXA
#define INDENTED_OSTREAM_H_TJD6GPXA

#include <ostream>
#include <iomanip>
#include <sstream>
struct indent_t {};

class indented_stringstream
{
  public:
    indented_stringstream()
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
      indent_level_ = level;
    }

    std::size_t indent_level() const
    {
      return indent_level_;
    }

    indented_stringstream& operator <<(indent_t)
    {
      if (indent_level() > 0)
        os_ << std::setw(indent_level()*2) << ' ';
      return *this;
    }

    template <typename T>
    indented_stringstream& operator <<(const T& t)
    {
      os_ << t;
      return *this;
    }

    std::string str() const {
      return os_.str();
    }

    void str(const char* s) {
      os_.str(s);
    }

    void clear() {
      os_.clear();
    }

  private:
    std::size_t indent_level_;
    std::stringstream os_;
};




#endif /* end of include guard: INDENTED_OSTREAM_H_TJD6GPXA */
