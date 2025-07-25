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

TEST_CASE("set optional field encoder_V2/decoder_v2","[set_optional_encoder_v2_decoder_v2]")
{
    SECTION("optional field present")
    {
        fast_test_coding_case_v2<simple19::templates_description> test_case;
        simple19::Test_3 test_3;
        simple19::Test_3_mref test_3_mref = test_3.mref();
        test_3_mref.set_field3().as(10);
        test_3_mref.set_TradeCondition().set_VolumeOnly();
        REQUIRE(test_3.cref().get_TradeCondition().present());
        REQUIRE(test_case.encoding(test_3.cref(), "\xF0\x83\x8A\x02\x81", true));
        REQUIRE(test_case.decoding("\xF0\x83\x8A\x02\x81", test_3.cref(), true));
    }

    SECTION("optional field not present")
    {
        fast_test_coding_case_v2<simple19::templates_description> test_case;
        simple19::Test_3 test_3;
        simple19::Test_3_mref test_3_mref = test_3.mref();
        test_3_mref.set_field3().as(10);
        REQUIRE(!test_3.cref().get_TradeCondition().present());
        REQUIRE(test_case.encoding(test_3.cref(), "\xE0\x83\x8A", true));
        REQUIRE(test_case.decoding("\xE0\x83\x8A", test_3.cref(), true));
    }
}
