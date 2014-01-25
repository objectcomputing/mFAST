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
#ifndef DEBUG_STREAM_H_E8L21B3
#define DEBUG_STREAM_H_E8L21B3

#include <iostream>
#include <mfast/output.h>
#ifdef NDEBUG

struct debug_stream {

  debug_stream()
  {
  }

  void set(std::ostream*) {
  }


  template <class T>
  const debug_stream& operator<<(const T&) const
  {
    return *this;
  }

  typedef std::ostream& (*ostream_manipulator)(std::ostream&);
  // define an operator<< to take in std::endl
  const debug_stream& operator<<(ostream_manipulator) const
  {
    // call the function, but we cannot return it's value
    return *this;
  }

};

#else

class debug_stream
{
  private:
    std::ostream* os_;
  public:
    debug_stream()
      : os_(0)
    {
    }

    void set(std::ostream* os)
    {
      os_ = os;
    }

    template <class T>
    const debug_stream& operator<<(const T& t ) const
    {
      if (os_ != 0) *os_ << t;
      return *this;
    }

    typedef std::ostream& (*ostream_manipulator)(std::ostream&);

    // define an operator<< to take in std::endl
    const debug_stream& operator<<(ostream_manipulator manip) const
    {
      // call the function, but we cannot return it's value
      if (os_ != 0) *os_ << manip;
      return *this;
    }

};

#endif

#endif /* end of include guard: DEBUG_STREAM_H_E8L21B3 */
