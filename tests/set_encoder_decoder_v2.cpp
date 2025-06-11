#include "catch.hpp"
#include <mfast.h>

#include "fast_test_coding_case_v2.hpp"
#include "byte_stream.h"

#include "simple19.h"

using namespace test::coding;

TEST_CASE("set encoder_V2/decoder_v2","[set_encoder_v2_decoder_v2]")
{
    fast_test_coding_case_v2<simple19::templates_description> test_case;
    simple19::Test_1 test_1;
    simple19::Test_1_mref test_1_mref = test_1.mref();

    test_1_mref.set_field1().as(10);

    auto TradeCondition = test_1_mref.set_TradeCondition();
    TradeCondition.set_OpeningPrice();
    TradeCondition.set_OfficialClosingPrice();

    REQUIRE(test_case.encoding(test_1.cref(),"\xF0\x81\x8A\x94",true));
    REQUIRE(test_case.decoding("\xF0\x81\x8A\x94",test_1.cref(),true));
}
