#include "catch.hpp"
#include <mfast.h>

#include "fast_test_coding_case_v2.hpp"
#include "byte_stream.h"

#include "simple19.h"

using namespace test::coding;

TEST_CASE("set data type basic test with default value")
{
    fast_test_coding_case_v2<simple19::templates_description> test_case;
    simple19::Test_1 test_1;

    {
        simple19::Test_1_mref test_1_mref = test_1.mref();
        auto TradeCondition = test_1_mref.set_TradeCondition();
        TradeCondition.set_OpeningPrice();
        TradeCondition.set_OfficialClosingPrice();
    }

    {
        simple19::Test_1_cref test_1_cref = test_1.cref();
        auto TradeCondition = test_1_cref.get_TradeCondition();

        REQUIRE(TradeCondition.value() == (0x00 | simple19::TradeConditionSet::ExchangeLast | simple19::TradeConditionSet::OpeningPrice | simple19::TradeConditionSet::OfficialClosingPrice));

        REQUIRE(TradeCondition.has_ExchangeLast());
        REQUIRE(TradeCondition.has_OpeningPrice());
        REQUIRE(TradeCondition.has_OfficialClosingPrice());
        REQUIRE(!TradeCondition.has_Retail());
    }
}

TEST_CASE("set data type basic test")
{
    fast_test_coding_case_v2<simple19::templates_description> test_case;
    simple19::Test_3 test_3;

    {
        simple19::Test_3_mref test_3_mref = test_3.mref();
        auto TradeCondition = test_3_mref.set_TradeCondition();
        TradeCondition.set_OpeningPrice();
        TradeCondition.set_OfficialClosingPrice();
    }

    {
        simple19::Test_3_cref test_3_cref = test_3.cref();
        auto TradeCondition = test_3_cref.get_TradeCondition();

        REQUIRE(TradeCondition.value() == (0x00 | simple19::TradeConditionSet::OpeningPrice | simple19::TradeConditionSet::OfficialClosingPrice));

        REQUIRE(!TradeCondition.has_ExchangeLast());
        REQUIRE(TradeCondition.has_OpeningPrice());
        REQUIRE(TradeCondition.has_OfficialClosingPrice());
        REQUIRE(!TradeCondition.has_Retail());
    }
}
