
#ifndef FAST_DECODER_V2_H_3FCCA80D
#define FAST_DECODER_V2_H_3FCCA80D

#include "decoder_v2/fast_decoder_core.h"


namespace mfast
{

class fast_decoder_v2
  : coder::fast_decoder_core<0>
{
public:

  template <typename DescriptionsTuple>
  fast_decoder_v2(const DescriptionsTuple& tp,
                  typename boost::disable_if< boost::is_base_of< mfast::templates_description, DescriptionsTuple>, allocator*>::type alloc = malloc_allocator::instance())
    : coder::fast_decoder_core<0>(alloc)
  {
    init(tp);
  }

  template <typename T>
  fast_decoder_v2(const T* desc,
                  typename boost::enable_if< boost::is_base_of< mfast::templates_description, T>, allocator*>::type alloc = malloc_allocator::instance())
    : coder::fast_decoder_core<0>(alloc)
  {
    init(boost::make_tuple(desc));
  }

  /// Decode a  message.
  ///
  /// @param[in,out] first The initial position of the buffer to be decoded. After decoding
  ///                the parameter is set to position of the first unconsumed data byte.
  /// @param[in] last The last position of the buffer to be decoded.
  /// @param[in] force_reset Force the decoder to reset and discard all exisiting history values.
  ///            Notice that the reset is done before the decoding of the input buffer rather than
  ///            after.
  message_cref decode(const char*& first, const char* last, bool force_reset = false)
  {
    return this->decode_stream(first, last, force_reset);
  }

};



} /* mfast */


#endif /* end of include guard: FAST_DECODER_V2_H_3FCCA80D */
