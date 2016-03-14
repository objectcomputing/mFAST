// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.
#pragma once

#include "int_ref.h"
#include "string_ref.h"
#include "decimal_ref.h"
#include <iostream>
#include <iomanip>
#include <boost/io/ios_state.hpp>

namespace mfast {
inline std::ostream &operator<<(std::ostream &os, const int32_cref &cref) {
  os << cref.value();
  return os;
}

inline std::ostream &operator<<(std::ostream &os, const uint32_cref &cref) {
  os << cref.value();
  return os;
}

inline std::ostream &operator<<(std::ostream &os, const int64_cref &cref) {
  os << cref.value();
  return os;
}

inline std::ostream &operator<<(std::ostream &os, const uint64_cref &cref) {
  os << cref.value();
  return os;
}

inline std::ostream &operator<<(std::ostream &os,
                                const ascii_string_cref &cref) {
  os << cref.c_str();
  return os;
}

inline std::ostream &operator<<(std::ostream &os,
                                const unicode_string_cref &cref) {
  os << cref.c_str();
  return os;
}

inline std::ostream &operator<<(std::ostream &os,
                                const byte_vector_cref &cref) {
  boost::io::ios_flags_saver ifs(os);
  os << std::hex << std::setfill('0');

  for (auto elem : cref) {
    os << std::setw(2) << static_cast<unsigned>(elem);
  }

  os << std::setfill(' ');
  return os;
}

inline std::ostream &operator<<(std::ostream &os, const decimal_cref &cref) {
  os << cref.value();
  return os;
}
}
