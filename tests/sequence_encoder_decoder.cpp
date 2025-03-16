#include "catch.hpp"
#include <mfast.h>

#include "fast_test_coding_case.hpp"
#include "byte_stream.h"

#include "simple14.h"

using namespace test::coding;

TEST_CASE("sequence optional copy operator by length encoder/decoder","[sequence_copy_operator_length_encoder_decoder]")
{
    fast_test_coding_case<simple14::templates_description> test_case;
    simple14::Test_1 test_1;
    simple14::Test_1_mref test_1_mref = test_1.mref();
    auto sequence_1_mref = test_1_mref.set_sequence_1();
    sequence_1_mref.resize(1);

    auto element_sequence = sequence_1_mref.front();
    element_sequence.set_field_1_2().as(50);

    test_1_mref.set_field_1_3().as(10);

    REQUIRE(test_case.encoding(test_1.cref(),"\xE0\x81\x82\xC0\xB3\x8B",true));
    REQUIRE(test_case.decoding("\xE0\x81\x82\xC0\xB3\x8B",test_1.cref(),true));
}

TEST_CASE("sequence optional constant operator by length encoder/decoder","[sequence_constant_operator_length_encoder_decoder]")
{
    fast_test_coding_case<simple14::templates_description> test_case;
    simple14::Test_2 test_2;
    simple14::Test_2 test_3;

    {
        simple14::Test_2_mref test_2_mref = test_2.mref();

        auto sequence_2_mref = test_2_mref.set_sequence_2();
        // First time hast to be initalized and the same value from the configuration xml
        sequence_2_mref.resize(1);

        auto element_sequence = sequence_2_mref.front();
        element_sequence.set_field_2_2().as(50);

        test_2_mref.set_field_2_3().as(10);

        REQUIRE(test_case.encoding(test_2.cref(),"\xE0\x82\xC0\xB3\x8B",true));
        REQUIRE(test_case.decoding("\xE0\x82\xC0\xB3\x8B",test_2.cref(),true));
    }
    {
        simple14::Test_2_mref test_3_mref = test_3.mref();
    
        auto sequence_3_mref = test_3_mref.set_sequence_2();
        sequence_3_mref.resize(2);

        {
            auto element_sequence = sequence_3_mref.front();
            element_sequence.set_field_2_2().as(50);
        }

        {
            auto element_sequence = sequence_3_mref.back();
            element_sequence.set_field_2_2().as(60);
        }

        test_3_mref.set_field_2_3().as(10);

        REQUIRE(test_case.encoding(test_3.cref(),"\xA0\x80\xC0\xBD\x8B"));
    }
}

TEST_CASE("sequence optional default operator by length encoder/decoder","[sequence_default_operator_length_encoder_decoder]")
{
    fast_test_coding_case<simple14::templates_description> test_case;
    simple14::Test_3 test_3;
    simple14::Test_3 test_4;

    {
        simple14::Test_3_mref test_3_mref = test_3.mref();

        auto sequence_3_mref = test_3_mref.set_sequence_3();
        // First time hast to be initalized and the same value from the configuration xml
        sequence_3_mref.resize(1);

        auto element_sequence = sequence_3_mref.front();
        element_sequence.set_field_3_2().as(50);

        test_3_mref.set_field_3_3().as(10);
    
        REQUIRE(test_case.encoding(test_3.cref(),"\xC0\x83\xC0\xB3\x8B",true));
        REQUIRE(test_case.decoding("\xC0\x83\xC0\xB3\x8B",test_3.cref(),true));
    }

    {
        simple14::Test_3_mref test_4_mref = test_4.mref();

        auto sequence_4_mref = test_4_mref.set_sequence_3();
        sequence_4_mref.resize(2);

        {
            auto element_sequence = sequence_4_mref.front();
            element_sequence.set_field_3_2().as(50);
        }

        {
            auto element_sequence = sequence_4_mref.front();
            element_sequence.set_field_3_2().as(60);
        }

        test_4_mref.set_field_3_3().as(10);

        REQUIRE(test_case.encoding(test_4.cref(),"\xA0\x83\xC0\xBD\xC0\x80\x8B"));
    }
}

TEST_CASE("sequence optional none operator by length encoder/decoder","[sequence_default_none_length_encoder_decoder]")
{
    fast_test_coding_case<simple14::templates_description> test_case;
    simple14::Test_4 test_4;
    simple14::Test_4_mref test_4_mref = test_4.mref();

    auto sequence_4_mref = test_4_mref.set_sequence_4();
    sequence_4_mref.resize(1);

    auto element_sequence = sequence_4_mref.front();
    element_sequence.set_field_4_2().as(50);

    test_4_mref.set_field_4_3().as(10);

    REQUIRE(test_case.encoding(test_4.cref(),"\xC0\x84\x82\xC0\xB3\x8B",true));
    REQUIRE(test_case.decoding("\xC0\x84\x82\xC0\xB3\x8B",test_4.cref(),true));
}

TEST_CASE("sequence inside sequence encoder/decoder","[sequence_inside_sequence_encoder_decoder]")
{
    fast_test_coding_case<simple14::templates_description> test_case;

    SECTION("outside sequence")
    {
        simple14::Test_5 test_5;
        simple14::Test_5_mref test_5_mref = test_5.mref();

        test_5_mref.set_field_5_2().as(1);

        auto sequence_5_1_mref = test_5_mref.set_sequence_5_1();
        sequence_5_1_mref.resize(1);

        {
            auto element_sequence = sequence_5_1_mref.front();
            element_sequence.set_field_5_4().as(50);
        }

        REQUIRE(test_case.encoding(test_5.cref(),"\xD0\x85\x81\x81\xC0\xB3",true));
        REQUIRE(test_case.decoding("\xD0\x85\x81\x81\xC0\xB3",test_5.cref(),true));
    }

    SECTION("inside sequence")
    {
        simple14::Test_5 test_5;
        simple14::Test_5_mref test_5_mref = test_5.mref();

        test_5_mref.set_field_5_2().as(1);

        auto sequence_5_1_mref = test_5_mref.set_sequence_5_1();
        sequence_5_1_mref.resize(1);

        {
            auto element_sequence = sequence_5_1_mref.front();
            element_sequence.set_field_5_4().as(50);

            auto sequence_5_2_mref = element_sequence.set_sequence_5_2();
            sequence_5_2_mref.resize(1);

            {
                auto element_sequence_inside = sequence_5_2_mref.front();
                element_sequence_inside.set_field_5_6().as(50);
            }
        }

        REQUIRE(test_case.encoding(test_5.cref(),"\xD0\x85\x81\x81\xE0\xB3\x82\xB2",true));
        REQUIRE(test_case.decoding("\xD0\x85\x81\x81\xE0\xB3\x82\xB2",test_5.cref(),true));
    }
}

TEST_CASE("group sequence inside sequence encoder/decoder","[group_sequence_inside_sequence_encoder_decoder]")
{
    fast_test_coding_case<simple14::templates_description> test_case;

    SECTION("outside sequence")
    {
        simple14::Test_6 test_6;
        simple14::Test_6_mref test_6_mref = test_6.mref();

        // set group
        {
            auto group_6 = test_6_mref.set_group_6();
            auto sequence_6_1_mref = group_6.set_sequence_6_1();
            sequence_6_1_mref.resize(1);

            //set sequence
            {
                auto element_sequence = sequence_6_1_mref.front();
                element_sequence.set_field_6_3().as(50);
            }
        }

        REQUIRE(test_case.encoding(test_6.cref(),"\xC0\x86\x81\xC0\xB2",true));
        REQUIRE(test_case.decoding("\xC0\x86\x81\xC0\xB2",test_6.cref(),true));
    }

    SECTION("inside sequence")
    {
        simple14::Test_6 test_6;
        simple14::Test_6_mref test_6_mref = test_6.mref();

        // set group
        {
            auto group_6 = test_6_mref.set_group_6();
            auto sequence_6_1_mref = group_6.set_sequence_6_1();
            sequence_6_1_mref.resize(1);

            //set sequence
            {
                auto element_sequence = sequence_6_1_mref.front();
                element_sequence.set_field_6_3().as(50);

                auto sequence_6_2_mref = element_sequence.set_sequence_6_2();
                sequence_6_2_mref.resize(1);

                {
                    auto element_sequence_inside = sequence_6_2_mref.front();
                    element_sequence_inside.set_field_6_6().as(50);
                }
            }
        }

        REQUIRE(test_case.encoding(test_6.cref(),"\xC0\x86\x81\xD0\xB2\x82\xB2",true));
        REQUIRE(test_case.decoding("\xC0\x86\x81\xD0\xB2\x82\xB2",test_6.cref(),true));
    }
}
