#include "catch.hpp"
#include <mfast.h>

#include "fast_test_coding_case_v2.hpp"
#include "byte_stream.h"

#include "simple13.h"

using namespace test::coding;

TEST_CASE("only field without group encoder_v2/decoder_v2","[field_without_group_encoder_v2_decoder_v2]")
{
    fast_test_coding_case_v2<simple13::templates_description> test_case;

    simple13::Test_1 test_1;
    simple13::Test_1_mref test_1_mref = test_1.mref();

    test_1_mref.set_field_1_1().as(30);
    test_1_mref.set_field_1_2().as(40);

    REQUIRE(test_case.encoding(test_1.cref(),"\xC0\x81\x9F\xA8",true));
    REQUIRE(test_case.decoding("\xC0\x81\x9F\xA8",test_1.cref(),true));
}

TEST_CASE("field with group encoder_v2/decoder_v2","[field_without_group_encoder_v2_decoder_v2]")
{
    fast_test_coding_case_v2<simple13::templates_description> test_case;

    simple13::Test_1 test_1;
    simple13::Test_1_mref test_1_mref = test_1.mref();

    test_1_mref.set_field_1_1().as(30);
    test_1_mref.set_field_1_2().as(40);

    auto group_1 = test_1_mref.set_group_1();
    group_1.set_field_1_4().as(10);

    REQUIRE(test_case.encoding(test_1.cref(),"\xE0\x81\x9F\xA8\x80\x8B",true));
    REQUIRE(test_case.decoding("\xE0\x81\x9F\xA8\x80\x8B",test_1.cref(),true));
}

TEST_CASE("optional group encoder_v2/decoder_v2","[optional_group_encoder_v2_decoder_v2]")
{
    fast_test_coding_case_v2<simple13::templates_description> test_case;

    SECTION("No group")
    {
        simple13::Test_4 test_4;

        REQUIRE(test_case.encoding(test_4.cref(),"\xC0\x84",true));
        REQUIRE(test_case.decoding("\xC0\x84",test_4.cref(),true));
    }

    SECTION("With group")
    {
        simple13::Test_4 test_4;
        simple13::Test_4_mref test_4_mref = test_4.mref();

        // set group
        {
            auto group_1 = test_4_mref.set_group_4_1();
            auto sequence_4_mref = group_1.set_sequence_4();
            sequence_4_mref.resize(1);
            auto element_sequence = sequence_4_mref.front();
            element_sequence.set_field_4_3().as(1);
        }

        REQUIRE(test_case.encoding(test_4.cref(),"\xE0\x84\x80\xC0\x81",true));
        REQUIRE(test_case.decoding("\xE0\x84\x80\xC0\x81",test_4.cref(),true));
    }

    SECTION("With group sequence and field")
    {
        simple13::Test_4 test_4;
        simple13::Test_4_mref test_4_mref = test_4.mref();

        // set group
        {
            auto group_1 = test_4_mref.set_group_4_1();
            auto sequence_4_mref = group_1.set_sequence_4();
            sequence_4_mref.resize(1);
            auto element_sequence = sequence_4_mref.front();
            element_sequence.set_field_4_3().as(1);

            group_1.set_field_4_4().as(10);
        }

        REQUIRE(test_case.encoding(test_4.cref(),"\xE0\x84\xC0\xC0\x81\x8B",true));
        REQUIRE(test_case.decoding("\xE0\x84\xC0\xC0\x81\x8B",test_4.cref(),true));
    }
}
