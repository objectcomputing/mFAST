#include "catch.hpp"
#include <mfast.h>

#include "fast_test_coding_case.hpp"
#include "byte_stream.h"

#include "simple13.h"

using namespace test::coding;

TEST_CASE("only field without group encoder/decoder","[field_without_group_encoder_decoder]")
{
    fast_test_coding_case<simple13::templates_description> test_case;

    simple13::Test_1 test_1;
    simple13::Test_1_mref test_1_mref = test_1.mref();

    test_1_mref.set_field_1_1().as(30);
    test_1_mref.set_field_1_2().as(40);

    REQUIRE(test_case.encoding(test_1.cref(),"\xC0\x81\x9F\xA8",true));
    REQUIRE(test_case.decoding("\xC0\x81\x9F\xA8",test_1.cref(),true));
}

TEST_CASE("field with group encoder/decoder","[field_without_group_encoder_decoder]")
{
    fast_test_coding_case<simple13::templates_description> test_case;

    simple13::Test_1 test_1;
    simple13::Test_1_mref test_1_mref = test_1.mref();

    test_1_mref.set_field_1_1().as(30);
    test_1_mref.set_field_1_2().as(40);

    auto group_1 = test_1_mref.set_group_1();
    group_1.set_field_1_4().as(10);

    REQUIRE(test_case.encoding(test_1.cref(),"\xE0\x81\x9F\xA8\x80\x8B",true));
    REQUIRE(test_case.decoding("\xE0\x81\x9F\xA8\x80\x8B",test_1.cref(),true));
}


