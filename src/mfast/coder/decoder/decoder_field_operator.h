// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.
#pragma once

#include "mfast/int_ref.h"
#include "mfast/string_ref.h"
#include "mfast/decimal_ref.h"
#include "mfast/vector_ref.h"
#include "fast_istream.h"

namespace mfast {
class decoder_field_operator {
public:
  virtual void decode(const int32_mref &mref, fast_istream &stream,
                      decoder_presence_map &pmap) const;
  virtual void decode(const uint32_mref &mref, fast_istream &stream,
                      decoder_presence_map &pmap) const;
  virtual void decode(const int64_mref &mref, fast_istream &stream,
                      decoder_presence_map &pmap) const;
  virtual void decode(const uint64_mref &mref, fast_istream &stream,
                      decoder_presence_map &pmap) const;
  virtual void decode(const ascii_string_mref &mref, fast_istream &stream,
                      decoder_presence_map &pmap) const;
  virtual void decode(const unicode_string_mref &mref, fast_istream &stream,
                      decoder_presence_map &pmap) const;
  virtual void decode(const byte_vector_mref &mref, fast_istream &stream,
                      decoder_presence_map &pmap) const;
  virtual void decode(const decimal_mref &mref, fast_istream &stream,
                      decoder_presence_map &pmap) const;
};

extern const decoder_field_operator *const decoder_operators[operators_count];
}
