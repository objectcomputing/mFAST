// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.
#pragma once

#ifndef REPORT_OVERFLOW
namespace mfast {
template <typename BaseType, typename DeltaType>
void check_overflow(BaseType, DeltaType, const field_instruction *,
                    fast_istream &) {}
}
#else

#include <stdint.h>
#include "mfast/fast_istream.h"
#include <boost/multiprecision/cpp_int.hpp>

namespace mfast {
typedef boost::multiprecision::int128_t int128_t;
inline void report_overflow(const field_instruction *instruction,
                            fast_istream &stream) {
  stream.warning_log() << "Instruction " << instruction->name()
                       << " id= " << instruction->id() << " overflow\n";
}

inline void check_overflow(int32_t base, int64_t delta,
                           const field_instruction *instruction,
                           fast_istream &stream) {
  int64_t r = base;
  r += delta;
  if (r > INT32_MAX || r < INT32_MIN)
    report_overflow(instruction, stream);
}

inline void check_overflow(uint32_t base, int64_t delta,
                           const field_instruction *instruction,
                           fast_istream &stream) {
  int64_t r = base;
  r += delta;
  if (r > UINT32_MAX || r < 0)
    report_overflow(instruction, stream);
}

inline void check_overflow(int64_t base, int64_t delta,
                           const field_instruction *instruction,
                           fast_istream &stream) {
  int128_t r = base;
  r += delta;
  if (r > INT64_MAX || r < INT64_MIN)
    report_overflow(instruction, stream);
}

inline void check_overflow(uint64_t base, int64_t delta,
                           const field_instruction *instruction,
                           fast_istream &stream) {
  int128_t r = base;
  r += delta;
  if (r > UINT64_MAX || r < 0)
    report_overflow(instruction, stream);
}

inline void check_overflow(int16_t base, int16_t delta,
                           const field_instruction *instruction,
                           fast_istream &stream) {
  int16_t r = base;
  r += delta;
  if (r > 63 || r < 63)
    report_overflow(instruction, stream);
}
}

#endif
