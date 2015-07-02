// Copyright (c) 2013, 2014, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
//
//     mFAST is free software: you can redistribute it and/or modify
//     it under the terms of the GNU Lesser General Public License as published
//     by
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
#pragma once

#include <vector>
#include <tuple>
#include "encoder_v2/fast_encoder_core.h"

namespace mfast {
///
class fast_encoder_v2 : coder::fast_encoder_core {
public:
  /// Consturct an encoder using the specified memory allocator
  template <typename Description1, typename... Descriptions>
  fast_encoder_v2(mfast::allocator *alloc, const Description1 *desc1,
                  const Descriptions *... rest)
      : coder::fast_encoder_core(alloc) {
    this->init(desc1, rest...);
  }

  /// Consturct an encoder using the default memory allocator ( i.e.
  /// mfast::malloc_allocator::instance() )
  template <typename Description1, typename... Descriptions,
            typename = enable_if_t<
                std::is_base_of<templates_description, Description1>::value>>
  fast_encoder_v2(const Description1 *desc1, const Descriptions *... rest)
      : coder::fast_encoder_core(mfast::malloc_allocator::instance()) {
    this->init(desc1, rest...);
  }

  /// Encode a  message into FAST byte stream.
  ///
  /// @param[in] message The message to be encoded.
  /// @param[in] buffer The start position for the encoded FAST stream to be
  /// written to.
  /// @param[in] buffer_size The capacity of @a buffer.
  /// @param[in] force_reset Force the encoder to reset and discard all
  /// exisiting history values.
  ///
  /// @returns The size of the encoded byte stream. If the supplied buffer size
  /// is smaller than
  /// requried,
  ///          an exception is thrown.
  std::size_t encode(const message_cref &message, char *buffer,
                     std::size_t buffer_size, bool force_reset = false) {
    return this->encode_i(message, buffer, buffer_size, force_reset);
  }

  /// Encode a  message into FAST byte stream and append the encoded stream to
  /// \a buffer.
  ///
  /// @param[in] message The message to be encoded.
  /// @param[in] buffer The buffer for the encoded FAST stream to be appended
  /// to.
  /// @param[in] force_reset Force the encoder to reset and discard all
  /// exisiting history values.
  void encode(const message_cref &message, std::vector<char> &buffer,
              bool force_reset = false) {
    this->encode_i(message, buffer, force_reset);
  }

  /// Instruct the encoder whether the overlong presence map is allowed.
  ///
  /// Overlong presence map is allowed by default for better performance.
  /// It can be disabled for better standard conformance reason.
  void allow_overlong_pmap(bool v) { this->allow_overlong_pmap_i(v); }
};
}
