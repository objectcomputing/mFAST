#include "catch.hpp"
#include <mfast.h>
#include <mfast/coder/fast_encoder.h>
#include <cstring>
#include <stdexcept>

#include "simple11.h"
#include "byte_stream.h"

TEST_CASE("test simple encoder","[test_encoder]")
{
    mfast::fast_encoder encoder_;
    encoder_.include({simple11::description()});

    const int buffer_size = 128;
    char buffer[buffer_size];

    simple11::Test_1 msg;
    simple11::Test_1_mref msg_ref = msg.mref();

    msg_ref.set_field1().as(1);
    msg_ref.set_field2().as(2);
    msg_ref.set_field3().as(3);

    std::size_t encoded_size = encoder_.encode(msg_ref, buffer, buffer_size);

    byte_stream result("\xF8\x81\x81\x82\x83");
    CHECK(result == byte_stream(buffer, encoded_size));
}
