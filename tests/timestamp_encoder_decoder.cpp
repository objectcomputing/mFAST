#include "catch.hpp"
#include <mfast.h>

#include "fast_test_coding_case.hpp"
#include "byte_stream.h"

#include "simple17.h"

using namespace test::coding;

TEST_CASE("timestamp test encoder/decoder","[timestamp_encoder_decoder]")
{
    fast_test_coding_case<simple17::templates_description> test_case;
    simple17::Test_1 test_1;
    simple17::Test_1_mref test_1_mref = test_1.mref();
    test_1_mref.set_field1().as(1);
    test_1_mref.set_TransactTime().as(2);
    REQUIRE(test_case.encoding(test_1.cref(),"\xF0\x81\x81\x83",true));
    REQUIRE(test_case.decoding("\xF0\x81\x81\x83",test_1.cref(),true));
}



