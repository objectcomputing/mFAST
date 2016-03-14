// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.// #include "mfast/output.h"
#include "fast_istream.h"

#include <boost/io/ios_state.hpp>

namespace mfast {
std::ostream &operator<<(std::ostream &os, const fast_istream &istream) {
  const char *ptr = istream.gptr();
  boost::io::ios_flags_saver ifs(os);

  for (int i = 0; i < 2; ++i) {
    // Output at most 2 stop bit encoded entities
    for (; ptr != istream.egptr(); ++ptr) {
      os << std::hex << std::setw(2) << std::setfill('0')
         << (static_cast<unsigned>(*ptr) & 0xFF) << " ";
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
