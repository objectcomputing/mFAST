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
#ifndef DECODER_FIELD_OPERATOR_H_NHLHKGSN
#define DECODER_FIELD_OPERATOR_H_NHLHKGSN

#include "mfast/int_ref.h"
#include "mfast/string_ref.h"
#include "mfast/decimal_ref.h"
#include "mfast/vector_ref.h"
#include "fast_istream.h"

namespace mfast {

  class decoder_field_operator
  {
  public:
    virtual void decode(const int32_mref&     mref,
                        fast_istream&         stream,
                        decoder_presence_map& pmap) const;
    virtual void decode(const uint32_mref&    mref,
                        fast_istream&         stream,
                        decoder_presence_map& pmap) const;
    virtual void decode(const int64_mref&     mref,
                        fast_istream&         stream,
                        decoder_presence_map& pmap) const;
    virtual void decode(const uint64_mref&    mref,
                        fast_istream&         stream,
                        decoder_presence_map& pmap) const;
    virtual void decode(const ascii_string_mref& mref,
                        fast_istream&            stream,
                        decoder_presence_map&    pmap) const;
    virtual void decode(const unicode_string_mref& mref,
                        fast_istream&              stream,
                        decoder_presence_map&      pmap) const;
    virtual void decode(const byte_vector_mref& mref,
                        fast_istream&           stream,
                        decoder_presence_map&   pmap) const;
    virtual void decode(const decimal_mref&   mref,
                        fast_istream&         stream,
                        decoder_presence_map& pmap) const;
  };

  extern const decoder_field_operator* const decoder_operators[operators_count];

}
#endif /* end of include guard: DECODER_FIELD_OPERATOR_H_NHLHKGSN */
