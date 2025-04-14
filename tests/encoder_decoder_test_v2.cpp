#include "catch.hpp"
#include <mfast.h>

#include "fast_test_coding_case_v2.hpp"
#include "byte_stream.h"

#include "simple12.h"

using namespace test::coding;

TEST_CASE("simple field and sequence optional encoder_v2/decoder_v2","[field_sequence_optional_sequence_encoder_v2_decoder2]")
{
    fast_test_coding_case_v2<simple12::templates_description> test_case;

    SECTION("encode field")
    {
        simple12::Test_1 test_1;
        simple12::Test_1_mref test_1_mref = test_1.mref();
        test_1_mref.set_field_1_1().as(1);
        // \xE0: 1110 : OK
        // 1 : Stop Bit
        // 1 : Set Template Id.
        // 1 : Set Field field_1_1
        // 0 : XXX
        REQUIRE(test_case.encoding(test_1.cref(),"\xE0\x81\x82\x80",true));
        REQUIRE(test_case.decoding("\xE0\x81\x82\x80",test_1.cref(),true));
    }

    SECTION("encode sequence")
    {
        simple12::Test_1 test_1;
        simple12::Test_1_mref test_1_mref = test_1.mref();

        auto sequence_1_mref = test_1_mref.set_sequence_1();
        sequence_1_mref.resize(1);
    
        auto element_sequence = sequence_1_mref.front();
        element_sequence.set_field_1_2().as(10);
        element_sequence.set_field_1_3().as(50);
        // \xC0: 1100 : OK
        // 1 : Stop Bit
        // 1 : Set Template Id.
        // 0 : Set Field field_1_1
        // 0 : XXX
        REQUIRE(test_case.encoding(test_1.cref(),"\xC0\x81\x82\x8B\xB3",true));
        REQUIRE(test_case.decoding("\xC0\x81\x82\x8B\xB3",test_1.cref(),true));
    }

    SECTION("encode field and sequence")
    {
        simple12::Test_1 test_1;
        simple12::Test_1_mref test_1_mref = test_1.mref();
        test_1_mref.set_field_1_1().as(1);
    
        auto sequence_1_mref = test_1_mref.set_sequence_1();
        sequence_1_mref.resize(1);

        auto element_sequence = sequence_1_mref.front();
        element_sequence.set_field_1_2().as(10);
        element_sequence.set_field_1_3().as(50);
        // \xE0: 1110 : OK
        // 1 : Stop Bit
        // 1 : Set Template Id.
        // 1 : Set Field field_1_1
        // 0 : XXX
        REQUIRE(test_case.encoding(test_1.cref(),"\xE0\x81\x82\x82\x8b\xB3",true));
        REQUIRE(test_case.decoding("\xE0\x81\x82\x82\x8b\xB3",test_1.cref(),true));
    }
}

TEST_CASE("simple field encoder_v2/decoder_v2","[field_optional_encoder_v2]")
{
    fast_test_coding_case_v2<simple12::templates_description> test_case;

    SECTION("encode fields")
    {
        simple12::Test_2 test_2;
        simple12::Test_2_mref test_2_mref = test_2.mref();
        test_2_mref.set_field_2_1().as(1);
        test_2_mref.set_field_2_2().as(2);
        // \xF0: 1111 : OK
        // 1 : Stop Bit
        // 1 : Set Template Id.
        // 1 : Set Field field_2_1
        // 1 : Set Field field_2_2
        REQUIRE(test_case.encoding(test_2.cref(),"\xF0\x82\x81\x82",true));
        REQUIRE(test_case.decoding("\xF0\x82\x81\x82",test_2.cref(),true));
    }
}

TEST_CASE("simple field and sequence encoder_v2/decoder_v2","[field_sequence_encoder_v2_decoder_v2]")
{
    fast_test_coding_case_v2<simple12::templates_description> test_case;

    SECTION("encode fields")
    {
        simple12::Test_3 test_3;
        simple12::Test_3_mref test_3_mref = test_3.mref();
        test_3_mref.set_field_3_1().as(1);

        auto sequence_3_mref = test_3_mref.set_sequence_3();
        sequence_3_mref.resize(1);

        auto element_sequence = sequence_3_mref.front();
        element_sequence.set_field_3_2().as(10);
        element_sequence.set_field_3_3().as(50);
        // \xE0: 1110 : OK
        // 1 : Stop Bit
        // 1 : Set Template Id.
        // 1 : Set Field field_3_1
        // 0 : XXX
        REQUIRE(test_case.encoding(test_3.cref(),"\xE0\x83\x81\x81\x8A\xB2",true));
        REQUIRE(test_case.decoding("\xE0\x83\x81\x81\x8A\xB2",test_3.cref(),true));
    }
}

TEST_CASE("simple field optional encoder_v2/decoder_v2","[field__optional_encoder_v2_decoder_v2]")
{
    fast_test_coding_case_v2<simple12::templates_description> test_case;

    SECTION("encode fields")
    {
        simple12::Test_4 test_4;
        simple12::Test_4_mref test_4_mref = test_4.mref();
        test_4_mref.set_field_4_1().as(1);
        test_4_mref.set_field_4_2().as(2);
        // \xF0 : 1111 : OK
        // 1 : Stop Bit
        // 1 : Set Template Id.
        // 1 : Set Field field_4_1
        // 1 : Set Field field_4_2
        REQUIRE(test_case.encoding(test_4.cref(),"\xF0\x84\x82\x83",true));
        REQUIRE(test_case.decoding("\xF0\x84\x82\x83",test_4.cref(),true));
    }

    SECTION("encode first optional field")
    {
        simple12::Test_4 test_4;
        simple12::Test_4_mref test_4_mref = test_4.mref();
        test_4_mref.set_field_4_1().as(1);
        // \xE0 : 1110 : OK
        // 1 : Stop Bit.
        // 1 : Set Template Id.
        // 1 : Set Field field_4_1
        // 0 : Set Field field_4_2
        REQUIRE(test_case.encoding(test_4.cref(),"\xE0\x84\x82",true));
        REQUIRE(test_case.decoding("\xE0\x84\x82",test_4.cref(),true));
    }

    SECTION("encode second optional field")
    {
        simple12::Test_4 test_4;
        simple12::Test_4_mref test_4_mref = test_4.mref();
        test_4_mref.set_field_4_2().as(2);
        // \xD0 : 1110 : OK
        // 1 : Stop Bit.
        // 1 : Set Template Id.
        // 0 : Set Field field_4_1
        // 1 : Set Field field_4_2
        REQUIRE(test_case.encoding(test_4.cref(),"\xD0\x84\x83",true));
        REQUIRE(test_case.decoding("\xD0\x84\x83",test_4.cref(),true));
    }
}

TEST_CASE("simple field group optional encoder_v2/decoder_v2","[field_group_optional_encoder_v2_decoder_v2]")
{
    fast_test_coding_case_v2<simple12::templates_description> test_case;

    SECTION("encode field")
    {
        simple12::Test_5 test_5;
        simple12::Test_5_mref test_5_mref = test_5.mref();
        test_5_mref.set_field_5_1().as(1);
        // \xE0 : 1110 : OK
        // 1 : Stop Bit.
        // 1 : Set Template Id.
        // 1 : Set Field field_5_1
        // 0 : Not Set test_5_group
        REQUIRE(test_case.encoding(test_5.cref(),"\xE0\x85\x82",true));
        REQUIRE(test_case.decoding("\xE0\x85\x82",test_5.cref(),true));
    }

    SECTION("encode group")
    {
        simple12::Test_5 test_5;
        simple12::Test_5_mref test_5_mref = test_5.mref();
        test_5_mref.omit_field_5_1();
        auto group_test_5_group = test_5_mref.set_test_5_group();
        auto group_test_5_sequence = group_test_5_group.set_sequence_5();
        group_test_5_sequence.resize(1);
        {
            auto element_sequence = group_test_5_sequence.front();
            element_sequence.set_field_5_2().as(1);
            element_sequence.set_field_5_3().as(2);
        }
        group_test_5_group.set_field_5_5().as(10);
        // \xD0 : 1110 : OK
        // 1 : Stop Bit.
        // 1 : Set Template Id.
        // 0 : Not Set Field field_5_1
        // 1 : Set Group test_5_group
        REQUIRE(test_case.encoding(test_5.cref(),"\xD0\x85\xC0\x81\x81\x82\x8B",true));
        REQUIRE(test_case.decoding("\xD0\x85\xC0\x81\x81\x82\x8B",test_5.cref(),true));
    }

    SECTION("encode field and group")
    {
        simple12::Test_5 test_5;
        simple12::Test_5_mref test_5_mref = test_5.mref();
        test_5_mref.set_field_5_1().as(1);
        auto group_test_5_group = test_5_mref.set_test_5_group();
        auto group_test_5_sequence = group_test_5_group.set_sequence_5();
        group_test_5_sequence.resize(1);
        {
            auto element_sequence = group_test_5_sequence.front();
            element_sequence.set_field_5_2().as(1);
            element_sequence.set_field_5_3().as(2);
        }
        group_test_5_group.set_field_5_5().as(10);
        // \xF0 : 1111 : OK
        // 1 : Stop Bit.
        // 1 : Set Template Id.
        // 1 : Set Field field_5_1
        // 1 : Set test_5_group
        REQUIRE(test_case.encoding(test_5.cref(),"\xF0\x85\x82\xC0\x81\x81\x82\x8B",true));
        REQUIRE(test_case.decoding("\xF0\x85\x82\xC0\x81\x81\x82\x8B",test_5.cref(),true));
    }
}

TEST_CASE("simple optional field with default value encoder_v2/decoder_v2","[optional_field_with_default_value_encoder_v2/decoder_v2]")
{
    fast_test_coding_case_v2<simple12::templates_description> test_case;

    SECTION("encode field")
    {
        simple12::Test_6 test_6;
        simple12::Test_6_mref test_6_mref = test_6.mref();
        test_6_mref.set_field_6_2().as(1);
        // \xD0 : 1110 : OK
        // 1 : Stop Bit.
        // 1 : Set Template Id.
        // 0 : Not Set Field field_6_1
        // 1 : Set field_6_2
        REQUIRE(test_case.encoding(test_6.cref(),"\xD0\x86\x82",true));
        test_6_mref.set_field_6_1().as(1);
        REQUIRE(test_case.decoding("\xD0\x86\x82",test_6.cref(),true));
    }
}

TEST_CASE("sequence optional with constant length encoder_v2/decoder_v2","[optional_sequence_with_constant_length_encoder_v2/decoder_v2]")
{
    fast_test_coding_case_v2<simple12::templates_description> test_case;

    SECTION("encode fields without sequence")
    {
        simple12::Test_7 test_7;
        simple12::Test_7_mref test_7_mref = test_7.mref();
        test_7_mref.set_field_7_1().as(1);
        REQUIRE(test_case.encoding(test_7.cref(),"\xE0\x87\x82",true));
        REQUIRE(test_case.decoding("\xE0\x87\x82",test_7.cref(),true));
    }

    SECTION("encode fields with sequence")
    {
        simple12::Test_7 test_7;
        simple12::Test_7_mref test_7_mref = test_7.mref();
        test_7_mref.set_field_7_1().as(1);

        {
            auto sequence_7_mref = test_7_mref.set_sequence_7();
            sequence_7_mref.resize(1);

            {
                auto element_sequence = sequence_7_mref.front();
                element_sequence.set_field_7_4().as(1);
                element_sequence.set_field_7_5().as(1);
            }
        }

        REQUIRE(test_case.encoding(test_7.cref(),"\xF0\x87\x82\xa0\x82\x82",true));
        REQUIRE(test_case.decoding("\xF0\x87\x82\xa0\x82\x82",test_7.cref(),true));
    }
}

TEST_CASE("decimal optional encoder_v2/decoder_v2","[decimal_optional_encoder_v2/decoder_v2]")
{
    fast_test_coding_case_v2<simple12::templates_description> test_case;

    SECTION("encode decimal value")
    {
        simple12::Test_8 test_8;
        simple12::Test_8_mref test_8_mref = test_8.mref();
        test_8_mref.set_field_8_1().as(15, 1);
        test_8_mref.set_field_8_2().as(20);

        REQUIRE(test_case.encoding(test_8.cref(),"\xE0\x88\x81\x8F\x94",true));
        REQUIRE(test_case.decoding("\xE0\x88\x81\x8F\x94",test_8.cref(),true));
    }
}
