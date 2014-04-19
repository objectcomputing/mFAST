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

#ifndef ENCODER_FIELD_OPERATOR_H_MNM3YM8X
#define ENCODER_FIELD_OPERATOR_H_MNM3YM8X

#include "mfast/int_ref.h"
#include "mfast/string_ref.h"
#include "mfast/decimal_ref.h"
#include "mfast/vector_ref.h"
#include "fast_ostream.h"

namespace mfast {

  class encoder_field_operator
  {
  public:
    virtual void encode(const int32_cref&     cref,
                        fast_ostream&         stream,
                        encoder_presence_map& pmap) const;
    virtual void encode(const uint32_cref&    cref,
                        fast_ostream&         stream,
                        encoder_presence_map& pmap) const;
    virtual void encode(const int64_cref&     cref,
                        fast_ostream&         stream,
                        encoder_presence_map& pmap) const;
    virtual void encode(const uint64_cref&    cref,
                        fast_ostream&         stream,
                        encoder_presence_map& pmap) const;
    virtual void encode(const ascii_string_cref& cref,
                        fast_ostream&            stream,
                        encoder_presence_map&    pmap) const;
    virtual void encode(const unicode_string_cref& cref,
                        fast_ostream&              stream,
                        encoder_presence_map&      pmap) const;
    virtual void encode(const byte_vector_cref& cref,
                        fast_ostream&           stream,
                        encoder_presence_map&   pmap) const;
    virtual void encode(const decimal_cref&   cref,
                        fast_ostream&         stream,
                        encoder_presence_map& pmap) const;
  };

  extern const encoder_field_operator* const encoder_operators[operators_count];

}

#endif /* end of include guard: ENCODER_FIELD_OPERATOR_H_MNM3YM8X */
