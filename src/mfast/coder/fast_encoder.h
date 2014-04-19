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

#ifndef ENCODER_H_PMUI0TYQ
#define ENCODER_H_PMUI0TYQ

#include "mfast_coder_export.h"
#include "mfast/message_ref.h"
#include "mfast/malloc_allocator.h"

#include <vector>

namespace mfast
{
struct fast_encoder_impl;

///
class MFAST_CODER_EXPORT fast_encoder
{
  public:
    /// Consturct a encoder using default memory allocator (i.e. malloc)
    fast_encoder(allocator* alloc = malloc_allocator::instance());

    ~fast_encoder();

    /// Import templates descriptions into the encoder.
    ///
    /// Notice that this encoder object does neither copy or hold the ownership of the passed
    /// description. The caller should ensure the lifetime of @a descriptions is longer than
    /// the encoder object.
    ///
    /// In addition, this memeber function should only be invoked once during the lifetime
    /// of a encoder object. Repetitive invoking the member function would produce undefined
    /// behavior.
    ///
    /// @param descriptions The array of templates_description pointers to be loaded.
    /// @param description_count Number of elements in @a descriptions array.
    void include(const templates_description** descriptions, std::size_t description_count);

    template<int N>
    void include(const templates_description* (&descriptions)[N])
    {
      include(descriptions, N);
    }

    const template_instruction* template_with_id(uint32_t id);
    /// Encode a  message into FAST byte stream.
    ///
    /// @param[in] message The message to be encoded.
    /// @param[in] buffer The start position for the encoded FAST stream to be written to.
    /// @param[in] buffer_size The capacity of @a buffer.
    /// @param[in] force_reset Force the encoder to reset and discard all exisiting history values.
    ///
    /// @returns The size of the encoded byte stream. If the supplied buffer size is smaller than requried,
    ///          an exception is thrown.
    std::size_t encode(const message_cref& message,
                       char*               buffer,
                       std::size_t         buffer_size,
                       bool                force_reset = false);

    /// Encode a  message into FAST byte stream and append the encoded stream to \a buffer.
    ///
    /// @param[in] message The message to be encoded.
    /// @param[in] buffer The buffer for the encoded FAST stream to be appended to.
    /// @param[in] force_reset Force the encoder to reset and discard all exisiting history values.
    void encode(const message_cref& message,
                std::vector<char>&  buffer,
                bool                force_reset = false);

    /// Instruct the encoder whether the overlong presence map is allowed.
    ///
    /// Overlong presence map is allowed by default for better performance.
    /// It can be disabled for better standard conformance reason.
    void allow_overlong_pmap(bool v);

  private:
    fast_encoder_impl* impl_;
};

}

#endif /* end of include guard: ENCODER_H_PMUI0TYQ */
