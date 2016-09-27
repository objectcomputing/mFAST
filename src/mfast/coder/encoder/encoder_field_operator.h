// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.
#pragma once

#include "../../int_ref.h"
#include "../../string_ref.h"
#include "../../decimal_ref.h"
#include "../../vector_ref.h"
#include "fast_ostream.h"

namespace mfast {
class encoder_field_operator {
public:
  virtual void encode(const int32_cref &cref, fast_ostream &stream,
                      encoder_presence_map &pmap) const;
  virtual void encode(const uint32_cref &cref, fast_ostream &stream,
                      encoder_presence_map &pmap) const;
  virtual void encode(const int64_cref &cref, fast_ostream &stream,
                      encoder_presence_map &pmap) const;
  virtual void encode(const uint64_cref &cref, fast_ostream &stream,
                      encoder_presence_map &pmap) const;
  virtual void encode(const ascii_string_cref &cref, fast_ostream &stream,
                      encoder_presence_map &pmap) const;
  virtual void encode(const unicode_string_cref &cref, fast_ostream &stream,
                      encoder_presence_map &pmap) const;
  virtual void encode(const byte_vector_cref &cref, fast_ostream &stream,
                      encoder_presence_map &pmap) const;
  virtual void encode(const decimal_cref &cref, fast_ostream &stream,
                      encoder_presence_map &pmap) const;
};

extern const encoder_field_operator *const encoder_operators[operators_count];
}
