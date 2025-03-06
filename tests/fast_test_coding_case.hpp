#pragma once

#include <mfast.h>
#include <mfast/coder/fast_encoder.h>
#include <mfast/coder/fast_decoder.h>
#include "byte_stream.h"

namespace test
{
namespace coding
{
template<typename DESC>
class fast_test_coding_case
{
    public:
        fast_test_coding_case()
        {
            encoder_.include({DESC::instance()});
            decoder_.include({DESC::instance()});
        }

        bool encoding(const mfast::message_cref& msg_ref, const byte_stream& result, bool reset=false)
        {
            const int buffer_size = 128;
            char buffer[buffer_size];

            std::size_t encoded_size = encoder_.encode(msg_ref,
                                                        buffer,
                                                        buffer_size,
                                                        reset);

            if (result == byte_stream(buffer, encoded_size))
                return true;

            INFO(  "Got \"" << byte_stream(buffer, encoded_size) << "\" instead." );
            return false;
        }

        bool decoding(const byte_stream& bytes, const mfast::message_cref& result, bool reset=false)
        {
            const char* first = bytes.data();
            mfast::message_cref msg = decoder_.decode(first, first+bytes.size(), reset);

            return (msg == result);
        }

    private:
        mfast::fast_encoder encoder_;
        mfast::fast_decoder decoder_;
};
} // namespace coding
} // namespace test
