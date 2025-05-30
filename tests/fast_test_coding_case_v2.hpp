#pragma once

#include <mfast.h>
#include <mfast/coder/fast_encoder_v2.h>
#include <mfast/coder/fast_decoder_v2.h>
#include "byte_stream.h"

namespace test
{
namespace coding
{
template<typename DESC>
class fast_test_coding_case_v2
{
    public:
        fast_test_coding_case_v2():
            encoder_v2_(DESC::instance()),
            decoder_v2_(DESC::instance())
        {}

        bool encoding(const mfast::message_cref& msg_ref, const byte_stream& result, bool reset=false)
        {
            const int buffer_size = 128;
            char buffer[buffer_size];

            std::size_t encoded_size = encoder_v2_.encode(msg_ref, buffer, buffer_size, reset);
            if (result == byte_stream(buffer, encoded_size))
              return true;

            INFO( "Got \"" << byte_stream(buffer, encoded_size) << "\" instead." );
            return false;
        }

        bool decoding(const byte_stream& bytes, const mfast::message_cref& result, bool reset=false)
        {
          const char* first = bytes.data();
          mfast::message_cref msg = decoder_v2_.decode(first, first+bytes.size(), reset);

          return (msg == result);
        }

    private:
        mfast::fast_encoder_v2 encoder_v2_;
        mfast::fast_decoder_v2<0> decoder_v2_;
};
} // namespace coding
} // namespace test
