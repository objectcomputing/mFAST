// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.
#pragma once

#include "decoder_v2/fast_decoder_core.h"
#include <type_traits>
#include <tuple>

template <typename... CONDITIONS> struct all_true;

template <> struct all_true<> : std::true_type {};

template <typename CONDITION, typename... CONDITIONS>
struct all_true<CONDITION, CONDITIONS...>
    : std::integral_constant<bool, CONDITION::value &&
                                       all_true<CONDITIONS...>::value> {};

template <typename... Ts>
using CheckAllTemplatesDescription = typename std::enable_if<
    all_true<std::is_base_of<mfast::templates_description, Ts>...>::value,
    int>::type;

namespace mfast {
///
///  FAST decoder class
///
/// The class is templatized by the number of tokens (NumTokens). A token is an
/// integer number
/// ranges from 0 to
/// NumTokens-1. Conceptually, a token represent the exclusive right to access a
/// particular section
/// of resources.
/// When NumTokens>0, the decode function requires a token parameter and the
/// returned message is
/// associated with the
/// token passed for decoding. Before the decode function returned, the token
/// been passed is
/// considered to be used.
/// After the decode functin returned, accessing the returned message is  also
/// considered using the
/// token. The design
/// of the API restrict that a token cannot be used conncurently at any given
/// time. That is to say,
/// given a token
/// value x, if x is used for decoding in one thread, no other thread can access
/// a message which is
/// previously decoded
/// with the same associated token x.
///
/// When NumTokens <=1, message decoding and accessing the returned decoded
/// message must be in
/// lock-step sequence, no
/// concurrency is allowed. The difference between NumTokens==0 and NumTokens==1
/// is that the decoded
/// message is
/// read only when NumTokens==0 while the decoded message is modfiable when
/// NumTokens==1. In the
/// case NumTokens==0,
/// the FAST dictionary values inside the decoder would be shared with the
/// returned messages.
/// Consequently, it
/// can avoid certain unnecessary data copying during decoding. However,
/// modifying the returned
/// message could destroy
/// the dictionary values essential to decoding subsequent messages. On the
/// other hand, the decoder
/// would
/// always duplicate the dictionay values when NumTokens>0 and thus modifying
/// the returned messages
/// is allowed.

template <unsigned NumTokens>
class fast_decoder_v2 : coder::fast_decoder_core<NumTokens> {
public:
  /// Consturct a decoder using the specified memory allocator
  template <typename Description1, typename... Descriptions>
  fast_decoder_v2(mfast::allocator *alloc, const Description1 *desc1,
                  const Descriptions *... rest)
      : coder::fast_decoder_core<NumTokens>(alloc) {
    this->init(desc1, rest...);
  }

  /// Consturct a decoder using the default memory allocator ( i.e.
  /// mfast::malloc_allocator::instance() )
  template <typename Description1, typename... Descriptions,
            typename = enable_if_t<
                std::is_base_of<templates_description, Description1>::value>>
  fast_decoder_v2(const Description1 *desc1, const Descriptions *... rest)
      : coder::fast_decoder_core<NumTokens>(
            mfast::malloc_allocator::instance()) {
    this->init(desc1, rest...);
  }

  fast_decoder_v2(const fast_decoder_v2 &) = delete;

  /// Decode a  message.
  ///
  /// @param[in] token The exclusive token value associated with the returned
  /// message.
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
  message_mref decode(std::size_t token, const char *&first, const char *last,
                      bool force_reset = false) {
    assert(token < NumTokens);
    return this->decode_stream(token, first, last, force_reset);
  }
};

template <> class fast_decoder_v2<0> : coder::fast_decoder_core<0> {
public:
  /// Consturct a decoder using the specified memory allocator
  template <typename Description1, typename... Descriptions>
  fast_decoder_v2(mfast::allocator *alloc, const Description1 *desc1,
                  const Descriptions *... rest)
      : coder::fast_decoder_core<0>(alloc) {
    this->init(desc1, rest...);
  }

  /// Consturct a decoder using the default memory allocator ( i.e.
  /// mfast::malloc_allocator::instance() )
  template <typename Description1, typename... Descriptions,
            typename = enable_if_t<
                std::is_base_of<templates_description, Description1>::value>>
  fast_decoder_v2(const Description1 *desc1, const Descriptions *... rest)
      : coder::fast_decoder_core<0>(mfast::malloc_allocator::instance()) {
    this->init(desc1, rest...);
  }

  fast_decoder_v2(const fast_decoder_v2 &) = delete;
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
                      bool force_reset = false) {
    return this->decode_stream(0, first, last, force_reset);
  }
};

} /* mfast */
