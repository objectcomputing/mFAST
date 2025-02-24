#include "catch.hpp"
#include <mfast.h>

#include "simple1.h"
#include "simple2.h"

TEST_CASE("test message pmap message number of bits of a field message content","[message_pmap_number_bits_field]")
{
    simple1::Test msg;
    simple1::Test_mref msg_ref = msg.mref();

    msg_ref.set_field1().as(1);
    msg_ref.set_field2().as(2);
    msg_ref.set_field3().as(3);

    CHECK(msg.instruction()->segment_pmap_size() == 4);
}

TEST_CASE("test message pmap message number of bits of a field and group message content","[message_pmap_number_bits_group]")
{
    simple2::Test msg;
    simple2::Test_mref msg_ref = msg.mref();

    msg_ref.set_field1().as(1);
    msg_ref.set_group1().set_field2().as(2);
    msg_ref.set_group1().set_field3().as(3);

    CHECK(msg.instruction()->segment_pmap_size() == 3);
}
