// Copyright (c) 2013, Huang-Ming Huang,  Object Computing, Inc.
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
#ifndef DECODER_H_WYWT9TG7
#define DECODER_H_WYWT9TG7
#include "mfast/fast_istream.h"
#include "mfast/message_ref.h"


namespace mfast
{

struct decoder_impl;

///
class decoder
{
  public:
    /// Consturct a decoder using default memory allocator (i.e. malloc)
    decoder();

    /// Construct a decode using custom memory allocator
    decoder(allocator& alloc);
    ~decoder();

    /// Import templates descriptions into the decoder.
    ///
    /// Notice that this decoder object does neither copy or hold the ownership of the passed
    /// description. The callee should ensure the lifetime of @a descriptions are longer than
    /// the decoder object.
    ///
    /// In addition, this memeber function should only be invoked once during the lifetime
    /// of a decoder object. Repetitive invoking the member function would produce incorrect
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
    /// Decode a  message.
    // message_cref decode(fast_istreambuf& sb, bool force_reset = false);
    
    /// Decode a  message.
    ///
    /// @param[in,out] first The initial position of the buffer to be decoded. After decoding
    ///                the parameter is set to position of the first unconsumed data byte.
    /// @param[in] last The last position of the buffer to be decoded.
    /// @param[in] force_reset Force the decoder to reset and discard all exisiting history values.
    message_cref decode(const char*& first, const char* last, bool force_reset = false);

    void debug_log(std::ostream* os);    
    void warning_log(std::ostream* os);

  private:
    decoder_impl* impl_;
};


}




#endif /* end of include guard: DECODER_H_WYWT9TG7 */
