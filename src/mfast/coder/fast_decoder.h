// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.
#pragma once

#include "mfast_coder_export.h"
#include "../message_ref.h"
#include "../malloc_allocator.h"
#include <initializer_list>

namespace mfast {
struct fast_decoder_impl;

///
class MFAST_CODER_EXPORT fast_decoder {
public:
  /// Construct a decoder using a specified memory allocator
  fast_decoder(allocator *alloc = malloc_allocator::instance());
  ~fast_decoder();

  /// Import templates descriptions into the decoder.
  ///
  /// Notice that this decoder object does neither copy or hold the ownership of
  /// the passed
  /// description. The caller should ensure the lifetime of @a descriptions is
  /// longer than
  /// the decoder object.
  ///
  /// In addition, this memeber function should only be invoked once during the
  /// lifetime
  /// of a decoder object. Repetitive invoking the member function would produce
  /// undefined
  /// behavior.
  ///
  /// @param descriptions The array of templates_description pointers to be
  /// loaded.
  /// @param description_count Number of elements in @a descriptions array.
  void include(const templates_description *const *descriptions,
               std::size_t description_count);

  template <int N>
  void include(const templates_description *(&descriptions)[N]) {
    include(descriptions, N);
  }

  void
  include(std::initializer_list<const templates_description *> descriptions) {
    include(descriptions.begin(), descriptions.size());
  }

  /// Decode a  message.
  ///
  /// @param[in,out] first The initial position of the buffer to be decoded.
  /// After decoding
  ///                the parameter is set to position of the first unconsumed
  ///                data byte.
  /// @param[in] last The last position of the buffer to be decoded.
  /// @param[in] force_reset Force the decoder to reset and discard all
  /// exisiting history values.
  ///            Notice that the reset is done before the decoding of the input
  ///            buffer rather than
  ///            after.
  message_cref decode(const char *&first, const char *last,
                      bool force_reset = false);

  void debug_log(std::ostream *os);
  void warning_log(std::ostream *os);

private:
  fast_decoder_impl *impl_;
};
}
