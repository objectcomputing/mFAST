// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.
#pragma once

#include <iostream>
#include "../../output.h"
#ifdef NDEBUG

struct debug_stream {
  debug_stream() {}
  void set(std::ostream *) {}
  template <class T> const debug_stream &operator<<(const T &) const {
    return *this;
  }

  typedef std::ostream &(*ostream_manipulator)(std::ostream &);
  // define an operator<< to take in std::endl
  const debug_stream &operator<<(ostream_manipulator) const {
    // call the function, but we cannot return it's value
    return *this;
  }
};

#else

class debug_stream {
private:
  std::ostream *os_;

public:
  debug_stream() : os_(nullptr) {}
  void set(std::ostream *os) { os_ = os; }
  template <class T> const debug_stream &operator<<(const T &t) const {
    if (os_ != nullptr)
      *os_ << t;
    return *this;
  }

  typedef std::ostream &(*ostream_manipulator)(std::ostream &);

  // define an operator<< to take in std::endl
  const debug_stream &operator<<(ostream_manipulator manip) const {
    // call the function, but we cannot return it's value
    if (os_ != nullptr)
      *os_ << manip;
    return *this;
  }
};

#endif
