// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.
#include "decoder_presence_map.h"

#ifndef NDEBUG

namespace mfast {
std::ostream &operator<<(std::ostream &os, const decoder_presence_map &pmap) {
  size_t mask = pmap.mask_ >> 1;
  if (mask == 0) {
    os << "0";
    return os;
  }

  os << (mask & pmap.cur_bitmap_ ? "J" : "Q");
  while ((mask >>= 1)) {
    os << (mask & pmap.cur_bitmap_ ? "1" : "0");
  }
  return os;
}
}
#elif _MSC_VER
// disable MSVC LNK4221
namespace {
char dummy;
};
#endif
